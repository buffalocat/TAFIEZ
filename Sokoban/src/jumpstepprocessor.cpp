#include "stdafx.h"
#include "jumpstepprocessor.h"

#include "roommap.h"
#include "player.h"
#include "car.h"
#include "snakeblock.h"
#include "animationmanager.h"

JumpStepProcessor::JumpStepProcessor(RoomMap* map, DeltaFrame* delta_frame, Player* player, AnimationManager* anims,
	std::vector<GameObject*>& fall_check, std::vector<GameObject*>& moving_blocks) :
	fall_check_{ fall_check }, moving_blocks_{ moving_blocks },
	map_{ map }, delta_frame_{ delta_frame }, player_{ player }, anims_{ anims } {}

JumpStepProcessor::~JumpStepProcessor() {}

void JumpStepProcessor::run() {
	Car* car = player_->car_riding();
	car->parent_->driven_ = true;
	if (!compute_push_component_tree(player_)) {
		car->parent_->driven_ = false;
		return;
	}
	perform_jump();
	car->parent_->driven_ = false;
}


// Try to push the block and build the resulting component tree
// Return whether block is able to move
bool JumpStepProcessor::compute_push_component_tree(GameObject* block) {
	if (!compute_push_component(block)) {
		return false;
	}
	std::vector<GameObject*> weak_links{};
	collect_moving_and_weak_links(block->push_comp(), weak_links);
	for (auto link : weak_links) {
		if (!compute_push_component_tree(link)) {
			broken_weak_links_.push_back(link);
		}
	}
	return true;
}

// Try to push the component containing block
// Return whether block is able to move
bool JumpStepProcessor::compute_push_component(GameObject* start_block) {
	if (PushComponent* comp = start_block->push_comp()) {
		return !comp->blocked_;
	}
	auto comp_unique = std::make_unique<PushComponent>();
	PushComponent* comp = comp_unique.get();
	push_comps_unique_.push_back(std::move(comp_unique));
	start_block->collect_sticky_component(map_, Sticky::Strong, comp);
	// Check above everything in the component
	for (auto* block : comp->blocks_) {
		if (!block->pushable_ && !block->driven_) {
			comp->blocked_ = true;
			break;
		}
		if (GameObject* above = map_->view(block->pos_ + Point3{0, 0, 1})) {
			if (above->pushable_ || above->driven_) {
				if (compute_push_component(above)) {
					comp->add_pushing(above->push_comp());
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
	return !comp->blocked_;
}


void JumpStepProcessor::collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links) {
	if (comp->moving_) {
		return;
	}
	comp->moving_ = true;
	for (GameObject* block : comp->blocks_) {
		moving_blocks_.push_back(block);
		if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(block)) {
			moving_snakes_.push_back(sb);
		}
		block->collect_sticky_links(map_, Sticky::Weak, weak_links);
	}
	for (PushComponent* above : comp->pushing_) {
		collect_moving_and_weak_links(above, weak_links);
	}
}

void JumpStepProcessor::perform_jump() {
	// The only blocks that need to be fall-checked are the broken weak links
	fall_check_ = broken_weak_links_;
	std::set<SnakeBlock*> link_add_check{};
	link_add_check.insert(moving_snakes_.begin(), moving_snakes_.end());
	for (auto sb : moving_snakes_) {
		sb->break_blocked_links(fall_check_, delta_frame_);
	}
	for (auto sb : moving_snakes_) {
		sb->collect_maybe_confused_neighbors(map_, link_add_check);
	}
	// MAP BECOMES INCONSISTENT HERE (potential ID overlap)
	// In this section of code, the map can't be viewed
	auto forward_moving_blocks = moving_blocks_;
	// TODO: put animation code somewhere else, if possible?
	if (anims_) {
		for (auto* block : forward_moving_blocks) {
			anims_->set_linear_animation(Direction::Up, block);
		}
		anims_->set_linear_animation_frames();
	}
	map_->batch_shift(std::move(forward_moving_blocks), { 0,0,1 }, true, delta_frame_);
	// MAP BECOMES CONSISTENT AGAIN HERE
	for (auto sb : link_add_check) {
		sb->check_add_local_links(map_, delta_frame_);
	}
}
