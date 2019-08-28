#include "stdafx.h"
#include "moveprocessor.h"

#include "common_constants.h"
#include "color_constants.h"

#include "gameobject.h"
#include "player.h"
#include "gatebody.h"
#include "delta.h"
#include "room.h"
#include "roommap.h"
#include "door.h"
#include "car.h"
#include "gate.h"

#include "snakeblock.h"

#include "playingstate.h"

#include "horizontalstepprocessor.h"
#include "fallstepprocessor.h"
#include "jumpstepprocessor.h"

MoveProcessor::MoveProcessor(PlayingState* playing_state, RoomMap* map, DeltaFrame* delta_frame, Player* player, bool animated) :
	playing_state_{ playing_state }, map_{ map }, delta_frame_{ delta_frame }, player_{ player }, animated_{ animated } {}

MoveProcessor::~MoveProcessor() {}

bool MoveProcessor::update() {
	if (--frames_ <= 0) {
		switch (state_) {
		case MoveStep::Horizontal:
		case MoveStep::Jump:
			// Even if no switch checks occur, the next frame chunk
			// cannot be skipped - movement animation must finish.
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
		case MoveStep::Waiting:
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

void MoveProcessor::reset_player_jump() {
	// If the player jumped immediately before this, make them gravitable again
	if (!player_->gravitable_) {
		player_->gravitable_ = true;
		delta_frame_->push(std::make_unique<ToggleGravitableDelta>(player_));
	}
}

bool MoveProcessor::try_move_horizontal(Point3 dir) {
	HorizontalStepProcessor(map_, delta_frame_, player_, dir, fall_check_, moving_blocks_).run();
	if (moving_blocks_.empty()) {
		return false;
	}
	state_ = MoveStep::Horizontal;
	frames_ = HORIZONTAL_MOVEMENT_FRAMES - SWITCH_RESPONSE_FRAMES;
	reset_player_jump();
	return true;
}

// Returns whether a color change occured
bool MoveProcessor::try_color_change() {
	Car* car = player_->car_bound(map_);
	if (!(car && car->cycle_color(false))) {
		return false;
	}
	if (auto snake = dynamic_cast<SnakeBlock*>(car->parent_)) {
		snake->remove_wrong_color_links(map_, delta_frame_);
		snake->check_add_local_links(map_, delta_frame_);
	}
	state_ = MoveStep::ColorChange;
	frames_ = COLOR_CHANGE_MOVEMENT_FRAMES;
	delta_frame_->push(std::make_unique<ColorChangeDelta>(car, true));
	add_neighbors_to_fall_check(car->parent_);
	reset_player_jump();
	return true;
}

// Returns whether the toggle was successful
bool MoveProcessor::try_toggle_riding() {
	if (!player_->toggle_riding(map_, delta_frame_, this)) {
		return false;
	}
	state_ = MoveStep::ToggleRiding;
	frames_ = TOGGLE_RIDING_MOVEMENT_FRAMES;
	fall_check_.push_back(player_);
	if (Car* car = player_->car_bound(map_)) {
		fall_check_.push_back(car->parent_);
	}
	reset_player_jump();
	return true;
}

bool MoveProcessor::try_jump() {
	Car* car = player_->car_riding();
	if (!car || (car->type_ != CarType::Hover)) {
		return false;
	}
	// We can't jump if we just jumped!
	if (!player_->gravitable_) {
		return false;
	}
	JumpStepProcessor(map_, delta_frame_, player_, fall_check_, moving_blocks_).run();
	if (moving_blocks_.empty()) {
		return false;
	}
	state_ = MoveStep::Jump;
	frames_ = JUMP_MOVEMENT_FRAMES - SWITCH_RESPONSE_FRAMES;
	player_->gravitable_ = false;
	delta_frame_->push(std::make_unique<ToggleGravitableDelta>(player_));
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
	raise_gates();
	map_->check_clear_flag_collected(delta_frame_);
	if (!skippable || delta_frame_->changed()) {
		state_ = MoveStep::PreFallSwitch;
		frames_ = SWITCH_RESPONSE_FRAMES;
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

void MoveProcessor::push_rising_gate(Gate* gate) {
	rising_gates_[gate->body_->pos_].push_back(gate);
}

void MoveProcessor::raise_gates() {
	for (auto& p : rising_gates_) {
		auto& vec = p.second;
		if (vec.size() > 1) {
			bool pushable = true;
			bool gravitable = true;
			for (auto* gate : vec) {
				GateBody* body = gate->body_;
				pushable &= body->pushable_;
				gravitable &= body->gravitable_;
				body->destroy(delta_frame_, CauseOfDeath::Collided);
			}
			// Corrupt GateBodies are snake-shaped, because one ingredient must ALWAYS be a snake
			// Whether they're persistent doesn't matter, because they don't function
			auto corrupt = std::make_unique<GateBody>(p.first, WHITE, pushable, gravitable, true, false, true);
			fall_check_.push_back(corrupt.get());
			map_->create_in_map(std::move(corrupt), true, delta_frame_);
		} else {
			vec[0]->raise_gate(map_, delta_frame_);
		}
	}
	rising_gates_.clear();
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
		door_state_ = DoorState::Voided;
		player_->destroy(delta_frame_, CauseOfDeath::Voided);
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