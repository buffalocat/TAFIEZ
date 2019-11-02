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
#include "car.h"
#include "gate.h"
#include "signaler.h"
#include "incinerator.h"

#include "snakeblock.h"

#include "playingstate.h"

#include "horizontalstepprocessor.h"
#include "fallstepprocessor.h"
#include "jumpstepprocessor.h"
#include "door.h"

MoveProcessor::MoveProcessor(PlayingState* playing_state, RoomMap* map, DeltaFrame* delta_frame, Player* player, bool animated) :
	playing_state_{ playing_state }, map_{ map }, delta_frame_{ delta_frame }, player_{ player }, animated_{ animated } {
	set_standing_door();
}

MoveProcessor::~MoveProcessor() {}

void MoveProcessor::set_standing_door() {
	if (player_) {
		Point3 door_pos;
		switch (player_->state()) {
		case PlayerState::Free:
		case PlayerState::Bound:
			door_pos = player_->shifted_pos({ 0, 0, -1 });
			break;
		case PlayerState::RidingNormal:
		case PlayerState::RidingHidden:
			door_pos = player_->shifted_pos({ 0, 0, -2 });
			break;
		}
		if (GameObject* door_obj = map_->view(door_pos)) {
			if (Door* door = dynamic_cast<Door*>(door_obj->modifier())) {
				standing_door_ = door;
			}
		}
	}
}

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
			break;
		}
		
	}
	// TODO: move this responsibility to a new class
	for (GameObject* block : moving_blocks_) {
		block->update_animation();
	}
	// End of move checks
	if (frames_ <= 0) {
		try_jump_refresh();
	}
	return frames_ <= 0;
}

void MoveProcessor::abort() {
	// TODO: Put this in the new animation managing class
	for (GameObject* block : moving_blocks_) {
		block->reset_animation();
	}
}

void MoveProcessor::reset_player_jump() {
	for (auto* player : map_->player_list()) {
		if (!player->gravitable_) {
			player->gravitable_ = true;
			fall_check_.push_back(player);
			delta_frame_->push(std::make_unique<ToggleGravitableDelta>(player));
		}
	}
}

bool MoveProcessor::try_move_horizontal(Point3 dir) {
	HorizontalStepProcessor(this, map_, delta_frame_, player_, dir, fall_check_, moving_blocks_).run();
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
		snake->remove_wrong_color_links(delta_frame_);
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
	// If another player just jumped, that has to be reset
	reset_player_jump();
	player_->gravitable_ = false;
	delta_frame_->push(std::make_unique<ToggleGravitableDelta>(player_));
	return true;
}

void MoveProcessor::try_jump_refresh() {
	if (!player_ || player_->gravitable_) {
		// The player is not in a jumped state
		return;
	}
	// If the player would fall, we can't refresh it yet
	player_->gravitable_ = true;
	if (FallStepProcessor(this, map_, nullptr, { player_ }).run(true)) {
		player_->gravitable_ = false;
		return;
	}
	delta_frame_->push(std::make_unique<ToggleGravitableDelta>(player_));
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
		FallStepProcessor(this, map_, delta_frame_, std::move(fall_check_)).run(false);
		fall_check_.clear();
	}
}

void MoveProcessor::perform_switch_checks(bool skippable) {
	delta_frame_->reset_changed();
	map_->alert_activated_listeners(delta_frame_, this);
	map_->reset_local_state();
	map_->check_signalers(delta_frame_, this);
	for (auto* switchable : activated_switchables_) {
		switchable->check_active_change(map_, delta_frame_, this);
	}
	activated_switchables_.clear();
	raise_gates();
	run_incinerators();
	map_->check_clear_flag_collected(delta_frame_);
	map_->free_unbound_players(delta_frame_);
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
		}
	}
	set_standing_door();
}

void MoveProcessor::push_rising_gate(Gate* gate) {
	rising_gates_[gate->body_->pos_].push_back(gate);
}

void MoveProcessor::run_incinerators() {
	for (auto* inc : alerted_incinerators_) {
		if (inc->state()) {
			Point3 pos_above = inc->pos_above();
			if (GameObject* above = map_->view(pos_above)) {
				if (above->id_ == GENERIC_WALL_ID) {
					map_->clear(pos_above);
				} else {
					map_->take_from_map(above, true, true, delta_frame_);
					above->destroy(this, CauseOfDeath::Incinerated, true);
				}
			}
		}
	}
	alerted_incinerators_.clear();
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
				body->destroy(this, CauseOfDeath::Collided, true);
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
	// Don't plan a door move if we loaded in on the door, or if we just used a door, or if we were already on the door
	if (!player_ || door_state_ != DoorState::None || door == standing_door_) {
		return;
	}
	if (door_state_ == DoorState::None && door->usable()) {
		// Also, it should probably be the responsibility of the objects/door, not the MoveProcessor
		// TODO: rethink these checks to be SAFE
		if (GameObject* above = map_->view(door->pos_above())) {
			if (player_ == above) {
				door_travelling_objs_.push_back({ player_, {} });
			} else if (Car* car = player_->car_riding()) {
				if (above->modifier() == car) {
					door_travelling_objs_.push_back({ player_, {} });
					door_travelling_objs_.push_back({ above, {} });
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
	if (playing_state_->can_use_door(entry_door_, &dest_room_, door_travelling_objs_, door_dest_grid_)) {
		if (dest_room_->map() == map_) {
			door_state_ = DoorState::AwaitingIntExit;
		} else {
			door_state_ = DoorState::AwaitingExtExit;
		}
		for (auto& obj : door_travelling_objs_) {
			map_->take_from_map(obj.raw, true, true, delta_frame_);
			add_neighbors_to_fall_check(obj.raw);
		}
		for (auto& obj : door_travelling_objs_) {
			if (auto* snake = dynamic_cast<SnakeBlock*>(obj.raw)) {
				snake->break_tangible_links(delta_frame_, fall_check_);
			}
		}
		frames_ = FALL_MOVEMENT_FRAMES;
		state_ = MoveStep::DoorMove;
	} else {
		entry_door_ = nullptr;
		door_travelling_objs_.clear();
		door_state_ = DoorState::None;
	}
}

struct BlockedPosChecker {
	RoomMap* map;
	bool operator()(std::vector<Point3>& pos_list) {
		for (Point3 pos : pos_list) {
			if (map->view(pos)) {
				return true;
			}
		}
		return false;
	}
};

void MoveProcessor::place_door_travelling_objects() {
	int num_exits = (int)door_dest_grid_.size();
	int num_objs = (int)door_travelling_objs_.size();
	std::vector<SnakeBlock*> moved_snakes{};
	if (num_exits == 1) { // Normal door motion
		for (int i = 0; i < num_objs; ++i) {
			GameObject* obj = door_travelling_objs_[i].raw;
			obj->abstract_put(door_dest_grid_[0][i], delta_frame_);
			map_->put_in_map(obj, true, true, delta_frame_);
			if (auto* snake = dynamic_cast<SnakeBlock*>(obj)) {
				moved_snakes.push_back(snake);
			}
		}
	} else { // Split through the door
		std::vector<Player*> new_players{};
		for (int i = 0; i < num_objs; ++i) {
			GameObject* obj = door_travelling_objs_[i].raw;
			for (int j = 0; j < num_exits; ++j) {
				auto dup = obj->duplicate(map_, delta_frame_);
				dup->abstract_put(door_dest_grid_[j][i], delta_frame_);
				add_to_fall_check(dup.get());
				if (auto* snake = dynamic_cast<SnakeBlock*>(dup.get())) {
					moved_snakes.push_back(snake);
				} else if (auto* player = dynamic_cast<Player*>(dup.get())) {
					new_players.push_back(player);
				}
				map_->create_in_map(std::move(dup), true, delta_frame_);
			}
		}
		// Make sure to move everything before destroying the originals
		for (int i = 0; i < num_objs; ++i) {
			door_travelling_objs_[i].raw->destroy(this, CauseOfDeath::Split, false);
		}
		for (auto* player : new_players) {
			player->validate_state(map_, delta_frame_);
			map_->player_cycle_->add_player(player, delta_frame_, false);
		}
	}
	for (auto* snake : moved_snakes) {
		snake->check_add_local_links(map_, delta_frame_);
	}
}

void MoveProcessor::try_int_door_exit() {
	door_dest_grid_.erase(std::remove_if(door_dest_grid_.begin(), door_dest_grid_.end(), BlockedPosChecker{ map_ }), door_dest_grid_.end());
	// Can't move
	if (door_dest_grid_.empty()) {
		frames_ = FALL_MOVEMENT_FRAMES;
		door_state_ = DoorState::AwaitingUnentry;
		state_ = MoveStep::DoorMove;
		return;
	// Normal door motion
	}
	place_door_travelling_objects();
	frames_ = FALL_MOVEMENT_FRAMES;
	door_state_ = DoorState::IntSucceeded;
	state_ = MoveStep::DoorMove;
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
		for (auto& obj : door_travelling_objs_) {
			if (auto* snake = dynamic_cast<SnakeBlock*>(obj.raw)) {
				snake->check_add_local_links(map_, delta_frame_);
			}
		}
		frames_ = FALL_MOVEMENT_FRAMES;
		door_state_ = DoorState::IntSucceeded;
		state_ = MoveStep::DoorMove;
	} else {
		door_state_ = DoorState::Voided;
		for (auto& obj : door_travelling_objs_) {
			obj.raw->destroy(this, CauseOfDeath::Voided, true);
		}
	}
}

void MoveProcessor::ext_door_exit() {
	delta_frame_->push(std::make_unique<RoomChangeDelta>(playing_state_, playing_state_->active_room()));
	map_->player_cycle_->remove_player(player_, delta_frame_);
	playing_state_->activate_room(dest_room_);
	map_ = dest_room_->map();
	map_->player_cycle_->add_player(player_, delta_frame_, true);
	place_door_travelling_objects();
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


RoomChangeDelta::RoomChangeDelta(PlayingState* state, Room* room) :
	state_{ state }, room_{ room } {}

RoomChangeDelta::~RoomChangeDelta() {}

void RoomChangeDelta::revert() {
	state_->activate_room(room_);
}


ToggleGravitableDelta::ToggleGravitableDelta(GameObject* obj) :
	obj_{ obj } {}

ToggleGravitableDelta::~ToggleGravitableDelta() {}

void ToggleGravitableDelta::revert() {
	obj_->gravitable_ = !obj_->gravitable_;
}


ColorChangeDelta::ColorChangeDelta(Car* car, bool undo) : car_{ car }, undo_{ undo } {}

ColorChangeDelta::~ColorChangeDelta() {}

void ColorChangeDelta::revert() {
	car_->cycle_color(undo_);
}