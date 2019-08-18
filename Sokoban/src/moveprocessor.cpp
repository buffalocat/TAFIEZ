#include "stdafx.h"
#include "moveprocessor.h"

#include "common_constants.h"

#include "gameobject.h"
#include "player.h"
#include "gatebody.h"
#include "delta.h"
#include "room.h"
#include "roommap.h"
#include "door.h"
#include "car.h"

#include "snakeblock.h"

#include "playingstate.h"

#include "horizontalstepprocessor.h"
#include "fallstepprocessor.h"

MoveProcessor::MoveProcessor(PlayingState* playing_state, RoomMap* map, DeltaFrame* delta_frame, Player* player, bool animated) :
	playing_state_{ playing_state }, map_{ map }, delta_frame_{ delta_frame }, player_{ player }, animated_{ animated } {}

MoveProcessor::~MoveProcessor() {}

// TODO: remove the switch, put all logic in HSP
bool MoveProcessor::try_move(Point3 dir) {
	switch (player_->state()) {
	case PlayerState::Bound:
		move_bound(dir);
		break;
	case PlayerState::Free:
	case PlayerState::RidingNormal:
	case PlayerState::RidingHidden:
		move_general(dir);
		break;
	case PlayerState::Dead:
		break;
	}
	if (moving_blocks_.empty()) {
		return false;
	}
	state_ = MoveStep::Horizontal;
	frames_ = HORIZONTAL_MOVEMENT_FRAMES - SWITCH_RESPONSE_FRAMES;
	return true;
}

// TODO: make bound motion more flexible (allow pushes!!)
void MoveProcessor::move_bound(Point3 dir) {
	// When Player is Bound, no other agents move
	if (map_->view(player_->shifted_pos(dir))) {
		return;
	}
	// If the player is bound, it's on top of a block!
	auto* car = dynamic_cast<ColoredBlock*>(map_->view(player_->shifted_pos({ 0,0,-1 })));
	auto* adj = dynamic_cast<ColoredBlock*>(map_->view(car->shifted_pos(dir)));
	if (adj && car->color() == adj->color()) {
		map_->take_from_map(player_, false, true, nullptr);
		player_->set_linear_animation(dir);
		delta_frame_->push(std::make_unique<MotionDelta>(player_, dir, map_));
		moving_blocks_.push_back(player_);
		player_->shift_pos_from_animation();
		map_->put_in_map(player_, false, true, nullptr);
	}
}

void MoveProcessor::move_general(Point3 dir) {
	HorizontalStepProcessor(map_, delta_frame_, player_, dir, fall_check_, moving_blocks_).run();
}

bool MoveProcessor::update() {
	if (--frames_ <= 0) {
		switch (state_) {
		case MoveStep::Horizontal:
			// Even if no switch checks occur, the next frame chunk
			// cannot be skipped - horizontal animation must finish.
			perform_switch_checks(false);
			break;
		case MoveStep::PostDoorInit:
			map_->set_initial_state_after_door(delta_frame_, this);
			// fallthrough
		case MoveStep::ToggleRiding:
		case MoveStep::FirstLoadInit:
		case MoveStep::PreFallSwitch:
		case MoveStep::ColorChange:
		case MoveStep::DoorMove:
			try_fall_step();
			// If nothing happens, skip the next forced wait.
			perform_switch_checks(true);
			break;
		default:
			break;
		}
	}
	// TODO: move this responsibility to a new class
	for (GameObject* block : moving_blocks_) {
		block->update_animation();
	}
	//update_gate_transitions();
	return frames_ <= 0;
}

void MoveProcessor::abort() {
	// TODO: Put this in the new animation managing class
	for (GameObject* block : moving_blocks_) {
		block->reset_animation();
	}
}

// Returns whether a color change occured
bool MoveProcessor::try_color_change() {
	Car* car = player_->car_bound(map_);
	if (!(car && car->cycle_color(false))) {
		return false;
	}
	if (auto snake = dynamic_cast<SnakeBlock*>(car->parent_)) {
		snake->update_links_color(map_, delta_frame_);
	}
	state_ = MoveStep::ColorChange;
	frames_ = COLOR_CHANGE_MOVEMENT_FRAMES;
	delta_frame_->push(std::make_unique<ColorChangeDelta>(car, true));
	add_neighbors_to_fall_check(car->parent_);
	return true;
}

// Returns whether the toggle was successful
bool MoveProcessor::try_toggle_riding() {
	if (!player_->toggle_riding(map_, delta_frame_, this)) {
		return false;
	}
	state_ = MoveStep::ToggleRiding;
	frames_ = TOGGLE_RIDING_MOVEMENT_FRAMES;
	return true;
}

void MoveProcessor::add_neighbors_to_fall_check(GameObject* obj) {
	fall_check_.push_back(obj);
	for (Point3 d : DIRECTIONS) {
		if (GameObject* adj = map_->view(obj->shifted_pos(d))) {
			fall_check_.push_back(adj);
		}
	}
}

void MoveProcessor::try_fall_step() {
	moving_blocks_.clear();
	if (!fall_check_.empty()) {
		FallStepProcessor(map_, delta_frame_, std::move(fall_check_)).run();
		fall_check_.clear();
	}
}

void MoveProcessor::perform_switch_checks(bool skippable) {
	delta_frame_->reset_changed();
	map_->alert_activated_listeners(delta_frame_, this);
	map_->reset_local_state();
	map_->check_signalers(delta_frame_, this);
	map_->check_clear_flag_collected(delta_frame_);
	if (!skippable || delta_frame_->changed()) {
		state_ = MoveStep::PreFallSwitch;
		frames_ = FALL_MOVEMENT_FRAMES;
	} else {
		state_ = MoveStep::Done;
		switch (door_state_) {
		case DoorState::None:
			break;
		case DoorState::AwaitingEntry:
			try_door_entry();
			break;
		case DoorState::AwaitingIntExit:
			try_int_door_exit();
			break;
		case DoorState::AwaitingExtExit:
			ext_door_exit();
			break;
		case DoorState::AwaitingUnentry:
			try_door_unentry();
			break;
		default:
			break;
		}
	}
}

void MoveProcessor::plan_door_move(Door* door) {
	// Don't plan a door move if we loaded in on the door, or if we just used a door
	if (!player_ || door_state_ != DoorState::None) {
		return;
	}
	if (door_state_ == DoorState::None && door->usable()) {
		// Also, it should probably be the responsibility of the objects/door, not the MoveProcessor
		// TODO: rethink these checks to be SAFE
		if (GameObject* above = map_->view(door->pos_above())) {
			if (player_ == above) {
				door_travelling_objs_.push_back({ player_, player_->pos_ });
			} else if (Car* car = player_->car_riding()) {
				if (above->modifier() == car) {
					door_travelling_objs_.push_back({ player_, player_->pos_ });
					door_travelling_objs_.push_back({ above, above->pos_ });
				}
			}
		}
		if (door_travelling_objs_.empty()) {
			return;
		}
		door_state_ = DoorState::AwaitingEntry;
		entry_door_ = door;
	}
}

void MoveProcessor::try_door_entry() {
	if (playing_state_->can_use_door(entry_door_, door_travelling_objs_, &dest_room_)) {
		if (dest_room_->map() == map_) {
			door_state_ = DoorState::AwaitingIntExit;
		} else {
			door_state_ = DoorState::AwaitingExtExit;
		}
		for (auto& obj : door_travelling_objs_) {
			map_->take_from_map(obj.raw, true, true, delta_frame_);
			add_neighbors_to_fall_check(obj.raw);
		}
		frames_ = FALL_MOVEMENT_FRAMES;
		state_ = MoveStep::DoorMove;
	} else {
		entry_door_ = nullptr;
		door_travelling_objs_.clear();
		door_state_ = DoorState::None;
	}
}

void MoveProcessor::try_int_door_exit() {
	bool can_move = true;
	for (auto& obj : door_travelling_objs_) {
		if (map_->view(obj.dest)) {
			can_move = false;
			break;
		}
	}
	if (can_move) {
		for (auto& obj : door_travelling_objs_) {
			add_to_fall_check(obj.raw);
			obj.raw->abstract_put(obj.dest, delta_frame_);
			map_->put_in_map(obj.raw, true, true, delta_frame_);
		}
		frames_ = FALL_MOVEMENT_FRAMES;
		door_state_ = DoorState::IntSucceeded;
		state_ = MoveStep::PostDoorInit;
	} else {
		frames_ = FALL_MOVEMENT_FRAMES;
		door_state_ = DoorState::AwaitingUnentry;
		state_ = MoveStep::DoorMove;
	}
}

void MoveProcessor::try_door_unentry() {
	bool can_move = true;
	for (auto& obj : door_travelling_objs_) {
		if (map_->view(obj.raw->pos_)) {
			can_move = false;
			break;
		}
	}
	if (can_move) {
		for (auto& obj : door_travelling_objs_) {
			add_to_fall_check(obj.raw);
			map_->put_in_map(obj.raw, true, true, delta_frame_);
		}
		frames_ = FALL_MOVEMENT_FRAMES;
		door_state_ = DoorState::IntSucceeded;
		state_ = MoveStep::PostDoorInit;
	} else {
		std::cout << "TERRIBLE THINGS HAPPENED! (we're in the void!)" << std::endl;
		door_state_ = DoorState::Voided;
	}
}

void MoveProcessor::ext_door_exit() {
	delta_frame_->push(std::make_unique<RoomChangeDelta>(playing_state_, playing_state_->active_room()));
	playing_state_->activate_room(dest_room_);
	map_ = dest_room_->map();
	for (auto& obj : door_travelling_objs_) {
		add_to_fall_check(obj.raw);
		obj.raw->abstract_put(obj.dest, delta_frame_);
		map_->put_in_map(obj.raw, true, true, delta_frame_);
	}
	playing_state_->snap_camera_to_player();
	frames_ = FALL_MOVEMENT_FRAMES;
	door_state_ = DoorState::ExtSucceeded;
	state_ = MoveStep::PostDoorInit;
}

// NOTE: could be dangerous if repeated calls are made
// Either make sure this doesn't happen, or check for presence here.
void MoveProcessor::add_to_moving_blocks(GameObject* obj) {
	moving_blocks_.push_back(obj);
}

void MoveProcessor::add_to_fall_check(GameObject* obj) {
	fall_check_.push_back(obj);
}

void MoveProcessor::set_initializer_state() {
	frames_ = 1;
	state_ = MoveStep::FirstLoadInit;
}