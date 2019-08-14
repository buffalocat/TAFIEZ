#include "stdafx.h"
#include "playingstate.h"

#include "graphicsmanager.h"

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

PlayingState::PlayingState() : GameState() {}

PlayingState::~PlayingState() {}

void PlayingState::main_loop() {
	if (!move_processor_) {
		delta_frame_ = std::make_unique<DeltaFrame>();
	}
	handle_input();
	room_->draw_at_player(gfx_, player_, true, false, false);
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
				room_->map()->reset_local_state();
				if (player_) {
					room_->set_cam_pos(player_->pos_, player_->cam_pos());
				}
			} else if (undo_stack_->non_empty()) {
				undo_stack_->pop();
				if (player_) {
					room_->set_cam_pos(player_->pos_, player_->cam_pos());
				}
			}
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
		create_child(std::make_unique<PauseState>(gfx_, this, global_.get()));
		return;
	}
	if (input_cooldown > 0) {
		return;
	}
	RoomMap* map = room_->map();
	// TODO: Make a real "death" flag/state
	// Don't allow other input if player is "dead"
	if (!dynamic_cast<Player*>(map->view(player_->pos_))) {
		return;
	}
	// Process normal gameplay input
	if (glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS) {
		player_->toggle_riding(map, delta_frame_.get());
		input_cooldown = MAX_COOLDOWN;
		return;
	}
	if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS) {
		move_processor_ = std::make_unique<MoveProcessor>(this, map, delta_frame_.get(), player_, true);
		if (move_processor_->color_change()) {
			input_cooldown = MAX_COOLDOWN;
			return;
		} else {
			move_processor_.reset(nullptr);
		}
	}
	for (auto p : MOVEMENT_KEYS) {
		if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
			move_processor_ = std::make_unique<MoveProcessor>(this, map, delta_frame_.get(), player_, true);
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
	room_ = proom->room.get();
	return true;
}

// TODO: use a real deltaframe in set_initial_state here!
void PlayingState::load_room_from_path(std::filesystem::path path, bool use_default_player) {
	MapFileI file{ path };
	std::string name = path.stem().string();
	auto room = std::make_unique<Room>(name);
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
	room->map()->set_initial_state(false);
	loaded_rooms_[name] = std::make_unique<PlayingRoom>(std::move(room));
}

bool PlayingState::can_use_door(Door* door, std::vector<DoorTravellingObj>& objs, Room** dest_room_ptr) {
	DoorData* data = door->data();
	if (!loaded_rooms_.count(data->dest)) {
		load_room(data->dest, false);
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

void PlayingState::snap_camera_to_player() {
	room_->set_cam_pos(player_->pos_, player_->cam_pos());
}

void PlayingState::make_subsave() {}

void PlayingState::world_reset() {}