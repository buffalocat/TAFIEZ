#include "stdafx.h"
#include "playingstate.h"

#include "graphicsmanager.h"
#include "fontmanager.h"
#include "stringdrawer.h"

#include "gameobject.h"
#include "savefile.h"
#include "gameobjectarray.h"
#include "delta.h"
#include "player.h"
#include "room.h"
#include "roommap.h"
#include "camera.h"
#include "moveprocessor.h"
#include "door.h"
#include "mapfile.h"
#include "car.h"

#include "realplayingstate.h"
#include "pausestate.h"

#include "common_constants.h"

const std::unordered_map<int, Point3> MOVEMENT_KEYS{
	{GLFW_KEY_RIGHT, {1, 0, 0}},
	{GLFW_KEY_LEFT,  {-1,0, 0}},
	{GLFW_KEY_DOWN,  {0, 1, 0}},
	{GLFW_KEY_UP,    {0,-1, 0}},
};

PlayingRoom::PlayingRoom(std::unique_ptr<Room> arg_room) :
	room{ std::move(arg_room) } {}

PlayingState::PlayingState(GameState* parent) : GameState(parent) {}

PlayingState::~PlayingState() {}

void PlayingState::main_loop() {
	if (!move_processor_) {
		delta_frame_ = std::make_unique<DeltaFrame>();
	}
	handle_input();
	room_->draw_at_player(player_, true, false, false);
	if (!move_processor_) {
		undo_stack_->push(std::move(delta_frame_));
	}
}

void PlayingState::handle_input() {
	static int input_cooldown = 0;
	static int undo_combo = 0;
	if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
		if (input_cooldown == 0) {
			++undo_combo;
			if (undo_combo < UNDO_COMBO_FIRST) {
				input_cooldown = UNDO_COOLDOWN_FIRST;
			} else if (undo_combo < UNDO_COMBO_SECOND) {
				input_cooldown = UNDO_COOLDOWN_SECOND;
			} else {
				input_cooldown = UNDO_COOLDOWN_FINAL;
			}
			if (move_processor_) {
				move_processor_->abort();
				move_processor_.reset(nullptr);
				delta_frame_->revert();
				delta_frame_ = std::make_unique<DeltaFrame>();
				if (player_) {
					snap_camera_to_player();
				}
			} else if (undo_stack_->non_empty()) {
				undo_stack_->pop();
				if (player_) {
					snap_camera_to_player();
				}
			}
			room_->map()->reset_local_state();
			set_death_text(player_->death());
			return;
		}
	} else {
		undo_combo = 0;
	}
	if (input_cooldown > 0) {
		--input_cooldown;
	}
	// Ignore all other input if an animation is occurring
	if (move_processor_) {
		if (move_processor_->update()) {
			move_processor_.reset(nullptr);
			undo_stack_->push(std::move(delta_frame_));
			delta_frame_ = std::make_unique<DeltaFrame>();
		} else {
			return;
		}
	}
	// You can pause with any cooldown, but not in a move
	if (glfwGetKey(window_, GLFW_KEY_P) == GLFW_PRESS) {
		create_child(std::make_unique<PauseState>(this));
		return;
	}
	
	set_death_text(player_->death());
	if (player_->death() != CauseOfDeath::None) {
		return;
	}

	if (input_cooldown > 0) {
		return;
	}
	RoomMap* map = room_->map();
	
	// Process normal gameplay input
	if (glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS) {
		create_move_processor();
		if (move_processor_->try_toggle_riding()) {
			input_cooldown = MAX_COOLDOWN;
			return;
		} else {
			move_processor_.reset(nullptr);
		}
	}
	if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS) {
		create_move_processor();
		if (move_processor_->try_color_change()) {
			input_cooldown = MAX_COOLDOWN;
			return;
		} else {
			move_processor_.reset(nullptr);
		}
	}
	for (auto p : MOVEMENT_KEYS) {
		if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
			create_move_processor();
			// p.second is direction of movement
			if (!move_processor_->try_move(p.second)) {
				move_processor_.reset(nullptr);
				return;
			}
			input_cooldown = MAX_COOLDOWN;
			Point3 pos_below;
			return;
		}
	}
}

void PlayingState::create_move_processor() {
	move_processor_ = std::make_unique<MoveProcessor>(this, room_->map(), delta_frame_.get(), player_, true);
}

Room* PlayingState::active_room() {
	return room_;
}

bool PlayingState::activate_room(Room* room) {
	return activate_room(room->name());
}

bool PlayingState::activate_room(std::string name) {
	if (!loaded_rooms_.count(name)) {
		if (!load_room(name, false)) {
			return false;
		}
	}
	auto* proom = loaded_rooms_[name].get();
	proom->changed = true;
	Room* new_room = proom->room.get();
	if (room_ != new_room) {
		new_room->zone_label_->init();
		text_->toggle_string_drawer(new_room->zone_label_.get(), true);
		// Remove the old labels (if there's more cleanup than this, it should be its own method)
		if (room_) {
			text_->toggle_string_drawer(room_->zone_label_.get(), false);
			if (auto* context_label = room_->context_label_.get()) {
				text_->toggle_string_drawer(context_label, false);
			}
		}
		room_ = new_room;
	}
	return true;
}

void PlayingState::load_room_from_path(std::filesystem::path path, bool use_default_player) {
	MapFileI file{ path };
	std::string name = path.stem().string();
	auto room = std::make_unique<Room>(this, name);
	if (use_default_player) {
		Player* loaded_player{};
		room->load_from_file(*objs_, file, global_.get(), &loaded_player);
		if (loaded_player) {
			player_ = loaded_player;
			player_->validate_state(room->map());
		}
	} else {
		room->load_from_file(*objs_, file, global_.get(), nullptr);
	}
	loaded_rooms_[name] = std::make_unique<PlayingRoom>(std::move(room));
}

bool PlayingState::can_use_door(Door* door, std::vector<DoorTravellingObj>& objs, Room** dest_room_ptr) {
	DoorData* data = door->data();
	// If you can't load the room, give up
	// A door to a nonexistent room is indistinguishable from a blocked door
	if (!loaded_rooms_.count(data->dest)) {
		if (!load_room(data->dest, false)) {
			return false;
		}
	}
	Room* dest_room = loaded_rooms_[data->dest]->room.get();
	*dest_room_ptr = dest_room;
	RoomMap* cur_map = room_->map();
	RoomMap* dest_map = dest_room->map();
	Point3 dest_pos_local = Point3{ data->pos } +Point3{ dest_room->offset_pos_ };
	for (auto& obj : objs) {
		obj.dest = dest_pos_local + obj.raw->pos_ - door->pos();
		if (dest_map->view(obj.dest)) {
			return false;
		}
	}
	return true;
}

void PlayingState::set_death_text(CauseOfDeath death) {
	static CauseOfDeath current_death = CauseOfDeath::None;
	if (death != current_death) {
		current_death = death;
		std::string death_str{};
		switch (death) {
		case CauseOfDeath::None:
			death_str = "";
			break;
		case CauseOfDeath::Fallen:
			death_str = "FALLEN";
			break;
		case CauseOfDeath::Split:
			death_str = "SPLIT";
			break;
		case CauseOfDeath::Voided:
			death_str = "VOIDED";
			break;
		case CauseOfDeath::Incinerated:
			death_str = "INCINERATED";
			break;
		}
		death_message_ = std::make_unique<IndependentStringDrawer>(
			text_->fonts_->get_font(Fonts::ABEEZEE, 108),
			glm::vec4(0.8, 0.1, 0.2, 1.0), death_str, 0.0f, 4);
		text_->toggle_string_drawer(death_message_.get(), true);
	}
}

void PlayingState::make_subsave() {}

void PlayingState::world_reset() {}

void PlayingState::snap_camera_to_player() {
	room_->set_cam_pos(player_->pos_, player_->cam_pos(), true, true);
}