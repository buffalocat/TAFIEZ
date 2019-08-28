#include "stdafx.h"
#include "horizontalstepprocessor.h"

#include "roommap.h"

#include "player.h"
#include "car.h"
#include "autoblock.h"
#include "puppetblock.h"

#include "snakeblock.h"

HorizontalStepProcessor::HorizontalStepProcessor(RoomMap* map, DeltaFrame* delta_frame, Player* player, Point3 dir,
	std::vector<GameObject*>& fall_check, std::vector<GameObject*>& moving_blocks) :
	fall_check_{ fall_check }, moving_blocks_{ moving_blocks },
	map_{ map }, delta_frame_{ delta_frame }, player_{ player }, dir_{ dir } {}

HorizontalStepProcessor::~HorizontalStepProcessor() {
	for (auto sb : moving_snakes_) {
		sb->reset_internal_state();
	}
	for (auto sb : strong_drags_) {
		sb->reset_internal_state();
	}
}

void HorizontalStepProcessor::run() {
	switch (player_->state()) {
	case PlayerState::Free:
		move_free();
		break;
	case PlayerState::Bound:
		move_bound();
		break;
	case PlayerState::RidingNormal:
	case PlayerState::RidingHidden:
		move_riding();
		break;
	default:
		break;
	}
}

void HorizontalStepProcessor::move_free() {
	if (compute_push_component_tree(player_)) {
		perform_horizontal_step();
	}
}

void HorizontalStepProcessor::move_bound() {
	if (!compute_push_component_tree(player_)) {
		return;
	}
	bool valid_move = false;
	// We can move if the block beneath us was pushed OR the one in front of us WASN'T
	auto* below = dynamic_cast<ColoredBlock*>(map_->view(player_->pos_ + Point3{ 0, 0, -1 }));
	if (auto* comp_below = below->push_comp()) {
		if (comp_below->moving_) {
			valid_move = true;
		}
	}
	if (auto* forward_below = dynamic_cast<ColoredBlock*>(map_->view(player_->pos_ + dir_ + Point3{ 0, 0, -1 }))) {
		if (forward_below->color_ == below->color_) {
			if (auto* comp_forward_below = forward_below->push_comp()) {
				if (!comp_forward_below->moving_) {
					valid_move = true;
				}
			} else {
				valid_move = true;
			}
		}
	}
	if (!valid_move) {
		moving_blocks_.clear();
		return;
	}
	perform_horizontal_step();
	if (!map_->view(player_->pos_ + Point3{ 0, 0, -1 })) {
		player_->set_free(delta_frame_);
	}
}

void HorizontalStepProcessor::move_riding() {
	// Initialize all agents, mark as driven
	Car* car = player_->car_riding();
	car->parent_->driven_ = true;
	for (AutoBlock* ab : map_->autos_) {
		ab->parent_->driven_ = true;
	}
	for (PuppetBlock* pb : map_->puppets_) {
		pb->parent_->driven_ = true;
	}
	// If the player can move, then PuppetBlocks will move; otherwise, set driven_ to false
	bool player_moved;
	if (player_->state() == PlayerState::RidingNormal) {
		player_moved = compute_push_component_tree(player_);
	} else { // When RidingHidden, the car represents the player
		player_moved = compute_push_component_tree(car->parent_);
	}
	if (player_moved) {
		for (PuppetBlock* pb : map_->puppets_) {
			compute_push_component_tree(pb->parent_);
		}
	} else {
		for (PuppetBlock* pb : map_->puppets_) {
			pb->parent_->driven_ = false;
		}
	}
	for (AutoBlock* ab : map_->autos_) {
		compute_push_component_tree(ab->parent_);
	}
	// Apply the step
	if (!moving_blocks_.empty()) {
		perform_horizontal_step();
	}
	// Reset driven flags
	car->parent_->driven_ = false;
	for (AutoBlock* ab : map_->autos_) {
		ab->parent_->driven_ = false;
	}
	if (player_moved) {
		for (PuppetBlock* pb : map_->puppets_) {
			pb->parent_->driven_ = false;
		}
	}
}


// Try to push the block and build the resulting component tree
// Return whether block is able to move
bool HorizontalStepProcessor::compute_push_component_tree(GameObject* block) {
	snakes_to_recheck_ = {};
	if (!compute_push_component(block)) {
		return false;
	}
	std::vector<GameObject*> weak_links{};
	// Ensures that snakes which were "pushed late" still drag their links
	for (auto snake : snakes_to_recheck_) {
		snake->collect_dragged_snake_links(map_, dir_, weak_links, false);
	}
	collect_moving_and_weak_links(block->push_comp(), weak_links);
	for (auto link : weak_links) {
		if (auto sb = dynamic_cast<SnakeBlock*>(link)) {
			if (!compute_push_component_tree(link)) {
				sb->dragged_ = false;
			}
		} else {
			if (!compute_push_component_tree(link)) {
				broken_weak_links_.push_back(link);
			}
		}
	}
	return true;
}

// Try to push the component containing block
// Return whether block is able to move
bool HorizontalStepProcessor::compute_push_component(GameObject* start_block) {
	if (PushComponent* comp = start_block->push_comp()) {
		if (!comp->blocked_) {
			if (auto* sb = dynamic_cast<SnakeBlock*>(start_block)) {
				sb->dragged_ = false;
				snakes_to_recheck_.push_back(sb);
				snake_strong_drag_check(sb);
			}
		}
		return !comp->blocked_;
	}
	auto comp_unique = std::make_unique<PushComponent>();
	PushComponent* comp = comp_unique.get();
	push_comps_unique_.push_back(std::move(comp_unique));
	start_block->collect_sticky_component(map_, Sticky::StrongStick, comp);
	// Check in front of everything in the component
	for (auto* block : comp->blocks_) {
		if (!block->pushable_ && !block->driven_) {
			comp->blocked_ = true;
			break;
		}
		if (GameObject* in_front = map_->view(block->pos_ + dir_)) {
			if (in_front->pushable_ || in_front->driven_) {
				if (compute_push_component(in_front)) {
					comp->add_pushing(in_front->push_comp());
				} else {
					// The thing we tried to push couldn't move
					comp->blocked_ = true;
					break;
				}
			} else {
				// The thing we tried to push wasn't pushable
				comp->blocked_ = true;
				break;
			}
		}
	}
	for (auto* block : comp->blocks_) {
		if (auto* sb = dynamic_cast<SnakeBlock*>(block)) {
			snake_strong_drag_check(sb);
		}
	}
	return !comp->blocked_;
}

void HorizontalStepProcessor::snake_strong_drag_check(SnakeBlock* block) {
	// A dragged block can't drag neighbors
	if (!block->dragged_) {
		auto* comp = block->push_comp();
		std::vector<GameObject*> cur_strong_drags{};
		block->collect_dragged_snake_links(map_, dir_, cur_strong_drags, true);
		for (auto* link : cur_strong_drags) {
			auto* s_link = static_cast<SnakeBlock*>(link);
			if (compute_push_component(link)) {
				strong_drags_.push_back(s_link);
				comp->add_pushing(link->push_comp());
			} else {
				s_link->dragged_ = false;
				comp->blocked_ = true;
			}
		}
	}
}


void HorizontalStepProcessor::collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links) {
	if (comp->moving_) {
		return;
	}
	comp->moving_ = true;
	for (GameObject* block : comp->blocks_) {
		moving_blocks_.push_back(block);
		if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(block)) {
			moving_snakes_.push_back(sb);
			if (!sb->dragged_) {
				sb->collect_dragged_snake_links(map_, dir_, weak_links, false);
			}
		} else {
			block->collect_sticky_links(map_, Sticky::WeakStick, weak_links);
		}
	}
	for (PushComponent* in_front : comp->pushing_) {
		collect_moving_and_weak_links(in_front, weak_links);
	}
}

void HorizontalStepProcessor::perform_horizontal_step() {
	// Any block which moved forward could have moved off a ledge
	fall_check_ = moving_blocks_;
	for (auto* block : moving_blocks_) {
		if (auto* above = map_->view(block->shifted_pos({ 0,0,1 }))) {
			fall_check_.push_back(above);
		}
	}
	fall_check_.insert(fall_check_.end(), broken_weak_links_.begin(), broken_weak_links_.end());
	std::set<SnakeBlock*> link_add_check{};
	link_add_check.insert(moving_snakes_.begin(), moving_snakes_.end());
	for (auto sb : moving_snakes_) {
		sb->break_blocked_links_horizontal(fall_check_, map_, delta_frame_, dir_);
	}
	SnakePuller snake_puller{ map_, delta_frame_, moving_blocks_, link_add_check, fall_check_ };
	for (auto sb : moving_snakes_) {
		sb->collect_maybe_confused_neighbors(map_, link_add_check);
		snake_puller.prepare_pull(sb);
	}
	// MAP BECOMES INCONSISTENT HERE (potential ID overlap)
	// In this section of code, the map can't be viewed
	auto forward_moving_blocks = moving_blocks_;
	// TODO: put animation code somewhere else, if possible?
	for (auto* block : forward_moving_blocks) {
		block->set_linear_animation(dir_);
	}
	snake_puller.perform_pulls();
	map_->batch_shift(std::move(forward_moving_blocks), dir_, true, delta_frame_);
	// MAP BECOMES CONSISTENT AGAIN HERE
	for (auto sb : link_add_check) {
		sb->check_add_local_links(map_, delta_frame_);
	}
}
