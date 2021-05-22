#include "stdafx.h"
#include "playingstate.h"

#include "graphicsmanager.h"
#include "animationmanager.h"
#include "fontmanager.h"
#include "stringdrawer.h"
#include "soundmanager.h"
#include "globalanimation.h"

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
#include "globalflagconstants.h"
#include "color_constants.h"


KeyStatus::KeyStatus(int code) : code_{ code }, press_{ false }, held_{ false }, status_{ false } {}

void KeyStatus::update(GameState* state) {
	bool cur = (state->key_pressed(code_));
	if (held_) {
		held_ = cur;
	} else if (cur) {
		press_ = true;
		held_ = true;
	}
}

void KeyStatus::consume() {
	status_ = press_ ;
	press_ = false;
}

const int MOVEMENT_KEYS[4] = { GLFW_KEY_RIGHT, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_UP, };
const Point3 MOVEMENT_DIRS[4] = { { 1, 0, 0 }, { 0, 1, 0 }, { -1, 0, 0 }, { 0, -1, 0 } };

PlayingRoom::PlayingRoom(std::unique_ptr<Room> arg_room) :
	room{ std::move(arg_room) } {}

PlayingState::PlayingState(GameState* parent, PlayingGlobalData* global) :
	GameState(parent), global_{ global } {
	anims_ = std::make_unique<AnimationManager>(&gfx_->particle_shader_, this, text_->ui_atlas_, sound_);
	current_death_ = CauseOfDeath::None;
	current_death_state_ = DeathState::Alive;
}

PlayingState::~PlayingState() {}

void PlayingState::main_loop() {
	if (!move_processor_) {
		delta_frame_ = std::make_unique<DeltaFrame>();
	}
	update_key_status();
	if (!mandatory_wait_) {
		handle_input();
	}
	// Update Graphics State and Animations
	gfx_->update();
	anims_->update();
	// Draw stuff
	room_->draw_at_player(player_doa(), true, false, false);
	gfx_->pre_object_rendering();
	gfx_->prepare_draw_objects();
	gfx_->draw_objects();
	anims_->view_dir_ = gfx_->view_dir();
	anims_->draw_fall_trails();
	anims_->draw_special();

	gfx_->pre_particle_rendering();
	anims_->render_particles();
	text_->draw();
	update_global_animation();
	gfx_->post_rendering();
	anims_->draw_cutscene();
	// If a move is not currently happening, try to push the current DeltaFrame
	// If it's trivial, nothing will happen
	if (!move_processor_) {
		undo_stack_->push(std::move(delta_frame_));
	}
}

void PlayingState::update_key_status() {
	car_ride_key_.update(this);
	color_change_key_.update(this);
	player_switch_key_.update(this);
}

void PlayingState::handle_input() {
	if (input_cooldown > 0) {
		--input_cooldown;
	}

	if (key_pressed(GLFW_KEY_Z)) {
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
				move_processor_.reset(nullptr);
				delta_frame_->revert(this);
				anims_->reset_temp();
				delta_frame_ = std::make_unique<DeltaFrame>();
				move_camera_to_player(true);
				anims_->sounds_->queue_sound(SoundName::UndoClick);
			} else if (undo_stack_->non_empty()) {
				undo_stack_->pop();
				move_camera_to_player(true);
				anims_->sounds_->queue_sound(SoundName::UndoClick);
			}
			gfx_->set_state(GraphicsState::None);
			room_->map()->reset_local_state();
			set_death_text();
			return;
		}
	} else {
		undo_combo = 0;
	}

	// Manual camera movement
	room_->camera()->handle_free_cam_input(this);

	// Ignore all other input if an animation is occurring
	if (move_processor_) {
		if (move_processor_->update()) {
			move_processor_.reset(nullptr);
			undo_stack_->push(std::move(delta_frame_));
			delta_frame_ = std::make_unique<DeltaFrame>();
			if (queued_autosave_) {
				make_subsave(SaveType::Auto, 0, queued_autosave_);
				queued_autosave_ = nullptr;
			}
		} else {
			return;
		}
	}

	car_ride_key_.consume();
	color_change_key_.consume();
	player_switch_key_.consume();

	// You can pause with any cooldown, but not in a move
	if (key_pressed(GLFW_KEY_P)) {
		create_child(std::make_unique<PauseState>(this));
		return;
	}
	
	RoomMap* map = room_->map();
	PlayerCycle* p_cycle = map->player_cycle_.get();
	Player* player = p_cycle->current_player();

	set_death_text();

	if (input_cooldown > 0) {
		return;
	}
	
	if (player_switch_key_.status_) {
		if (p_cycle->cycle_player(delta_frame_.get())) {
			input_cooldown = MAX_COOLDOWN;
			return;
		}
	}

	// Input beyond this point requires an alive player.
	if (!player) {
		return;
	}

	if (car_ride_key_.status_) {
		create_move_processor(player);
		if (move_processor_->try_toggle_riding()) {
			input_cooldown = MAX_COOLDOWN;
			return;
		} else {
			move_processor_.reset(nullptr);
		}
	}
	if (color_change_key_.status_) {
		create_move_processor(player);
		if (move_processor_->try_color_change()) {
			input_cooldown = MAX_COOLDOWN;
			return;
		} else {
			move_processor_.reset(nullptr);
		}
	}
	if (key_pressed(GLFW_KEY_SPACE)) {
		create_move_processor(player);
		if (move_processor_->try_jump()) {
			input_cooldown = MAX_COOLDOWN;
			return;
		} else {
			move_processor_.reset(nullptr);
		}
	}
	for (int i = 0; i < 4; ++i) {
		if (key_pressed(MOVEMENT_KEYS[i])) {
			const double HALF_PI = 1.57079632679;
			double angle = room_->camera()->get_rotation();
			i = (i + (int)((angle + 4.5 * HALF_PI) / HALF_PI)) % 4;
			create_move_processor(player);
			if (!move_processor_->try_move_horizontal(MOVEMENT_DIRS[i])) {
				move_processor_.reset(nullptr);
				return;
			}
			input_cooldown = MAX_COOLDOWN;
			return;
		}
	}
	/*
	auto zone_order = "HVCKFG7T2N0DO5AES4UQ69WZJ3LPYBMRI81X!";
	if (key_pressed(GLFW_KEY_G)) {
		static int hub_i = 0;
		if (hub_i < 5) {
			global_->add_flag(HUB_ACCESSED_GLOBAL_FLAGS[hub_i]);
			++hub_i;
			input_cooldown = MAX_COOLDOWN;
			return;
		}
	}
	if (key_pressed(GLFW_KEY_H)) {
		static int visit_i = 0;
		if (visit_i < 37) {
			global_->add_flag(FLAG_COLLECT_FLAGS[get_clear_flag_index(zone_order[visit_i])]);
			++visit_i;
			input_cooldown = MAX_COOLDOWN;
			return;
		}
	}
	if (key_pressed(GLFW_KEY_J)) {
		static int flag_i = 0;
		if (flag_i < 37) {
			global_->add_flag(ZONE_ACCESSED_GLOBAL_FLAGS[get_clear_flag_index(zone_order[flag_i])]);
			++flag_i;
			input_cooldown = MAX_COOLDOWN;
			return;
		}
	}
	if (key_pressed(GLFW_KEY_K)) {
		static int x_alt_i = 0;
		if (x_alt_i < 4) {
			global_->add_flag(X_ALT_ACCESSED_GLOBAL_FLAGS[x_alt_i]);
			++x_alt_i;
			input_cooldown = MAX_COOLDOWN;
			return;
		}
	}
	if (key_pressed(GLFW_KEY_L)) {
		static int hub_alt_i = 0;
		if (hub_alt_i < 5) {
			global_->add_flag(HUB_ALT_ACCESSED_GLOBAL_FLAGS[hub_alt_i]);
			++hub_alt_i;
			input_cooldown = MAX_COOLDOWN;
			return;
		}
	}
	*/
}

void PlayingState::create_move_processor(Player* player) {
	move_processor_ = std::make_unique<MoveProcessor>(this, room_->map(), delta_frame_.get(), player, true);
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
		anims_->reset();
		new_room->zone_label_->init();
		text_->toggle_string_drawer(new_room->zone_label_.get(), true);
		// Remove the old labels (if there's more cleanup than this, it should be its own method)
		if (room_) {
			text_->toggle_string_drawer(room_->zone_label_.get(), false);
			if (auto* context_label = room_->context_label_.get()) {
				text_->toggle_string_drawer(context_label, false);
				room_->should_update_label_ = true;
			}
		}
		room_ = new_room;
		room_->map()->set_initial_state(this);
	}
	return true;
}

void PlayingState::load_room_from_path(std::filesystem::path path, bool use_default_player) {
	MapFileI file{ path };
	std::string name = path.stem().string();
	auto room = std::make_unique<Room>(this, name);
	RoomInitData init_data{};
	room->load_from_file(*objs_, file, global_, &init_data);
	Player* default_player = init_data.default_player;
	if (use_default_player && default_player) {
		room->map()->player_cycle_->set_active_player(init_data.default_player);
	} else {
		if (Player* active_player = init_data.active_player) {
			room->map()->player_cycle_->set_active_player(active_player);
		}
		// Uncreate the default objects we put in the map
		if (default_player) {
			room->map()->player_cycle_->remove_player(default_player, nullptr);
			room->map()->take_from_map(default_player, true, false, nullptr);
			room->map()->remove_from_object_array(default_player);
		}
		if (Car* car = init_data.default_car) {
			room->map()->remove_moved_car(car);
			room->map()->take_from_map(car->parent_, true, false, nullptr);
			room->map()->remove_from_object_array(car->parent_);
		}
	}
	room->map()->validate_players(nullptr);
	loaded_rooms_[name] = std::make_unique<PlayingRoom>(std::move(room));
}

void PlayingState::make_subsave(SaveType type, unsigned int, AutosavePanel*) {}

bool PlayingState::can_use_door(Door* ent_door, Room** dest_room_ptr,
	std::vector<DoorTravellingObj>& objs,
	std::vector<Door*>& exit_doors) {
	DoorData* data = ent_door->data();
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
	Point3 ent_door_pos = ent_door->pos();
	for (auto* dest_door : dest_map->door_groups_[data->id]) {
		std::vector<Point3> dest_pos_list{};
		Point3 exit_door_pos = dest_door->pos();
		bool door_ok = true;
		for (auto& obj : objs) {
			Point3 dest_pos = exit_door_pos + obj.rel_pos;
			if (dest_map->view(dest_pos)) {
				door_ok = false;
				break;
			}
		}
		if (door_ok) {
			exit_doors.push_back(dest_door);
		}
	}
	return exit_doors.size() > 0;
}

void PlayingState::set_death_text() {
	auto death = player_doa()->death();
	if (death != current_death_) {
		current_death_ = death;
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
			glm::vec4(0.8, 0.1, 0.2, 1.0), death_str, DEATH_STRING_HEIGHT, DEATH_STRING_FADE_FRAMES, 0.0f);
		text_->toggle_string_drawer(death_message_.get(), true);
	}
	DeathState death_state = DeathState::Alive;
	std::string death_substr{};
	if (death != CauseOfDeath::None) {
		if (room_->map()->player_cycle_->any_player_alive()) {
			death_state = DeathState::DeadCanSwitch;
		} else {
			death_state = DeathState::DeadAlone;
		}
	}
	if (death_state != current_death_state_) {
		current_death_state_ = death_state;
		switch (death_state) {
		case DeathState::Alive:
			death_substr = "";
			break;
		case DeathState::DeadAlone:
			death_substr = "Press Z to undo";
			break;
		case DeathState::DeadCanSwitch:
			death_substr = "Press V to switch";
			break;
		}
		death_submessage_ = std::make_unique<IndependentStringDrawer>(
			text_->fonts_->get_font(Fonts::ABEEZEE, 48),
			glm::vec4(0.8, 0.1, 0.2, 1.0), death_substr, DEATH_SUBSTRING_HEIGHT, DEATH_STRING_FADE_FRAMES, 0.0f);
		text_->toggle_string_drawer(death_submessage_.get(), true);
	}
}


void PlayingState::move_camera_to_player(bool snap) {
	Player* player = player_doa();
	room_->set_cam_pos(player->pos_, player->cam_pos(), true, snap);
}

// The current player, dead or alive
Player* PlayingState::player_doa() {
	PlayerCycle* pc = room_->map()->player_cycle_.get();
	if (auto* player = pc->current_player()) {
		return player;
	} else {
		return pc->dead_player();
	}
}

void PlayingState::update_global_animation() {
	if (global_anim_) {
		if (global_anim_->update()) {
			global_anim_.reset();
		}
	}
}

void PlayingState::handle_escape() {
	auto pause_state = std::make_unique<PauseState>(this);
	pause_state->can_escape_quit_ = false;
	create_child(std::move(pause_state));
}