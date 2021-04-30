#include "stdafx.h"
#include "realplayingstate.h"

#include "graphicsmanager.h"
#include "animationmanager.h"
#include "common_constants.h"
#include "gameobjectarray.h"
#include "room.h"
#include "roommap.h"
#include "savefile.h"
#include "mapfile.h"
#include "delta.h"
#include "player.h"
#include "globalflagconstants.h"

RealPlayingState::RealPlayingState(SaveProfile* savefile, GameState* parent) :
	PlayingState(parent, savefile->global_.get()), savefile_{ savefile } {}

RealPlayingState::~RealPlayingState() {
	if (delta_frame_) {
		delta_frame_->revert(this);
	}
	savefile_->make_emergency_save(this);
}

void RealPlayingState::play_from_map(std::string starting_map) {
	load_room(starting_map, true);
	activate_room(starting_map);
	move_camera_to_player(true);
	room_->map()->set_initial_state(this);
	gfx_->set_state(GraphicsState::FadeIn);
}

bool RealPlayingState::load_room(std::string name, bool use_default_player) {
	bool from_main;
	std::filesystem::path path = savefile_->get_path(name, &from_main);
	if (!std::filesystem::exists(path)) {
		return false;
	}
	load_room_from_path(path, use_default_player);
	return true;
}

void RealPlayingState::make_subsave(SaveType type, unsigned int save_index, AutosavePanel* panel) {
	if (player_doa()->death_ != CauseOfDeath::None) {
		undo_stack_->pop();
	}
	switch (type) {
	case SaveType::Emergency:
		savefile_->make_emergency_save(this);
		break;
	case SaveType::Auto:
		savefile_->make_auto_save(panel, this);
		break;
	case SaveType::Manual:
		savefile_->make_manual_save(save_index, this);
		break;
	}
}

void RealPlayingState::play_from_loaded_subsave() {
	std::string cur_room_name = savefile_->cur_room_name_;
	load_room(cur_room_name, false);
	activate_room(cur_room_name);
	move_camera_to_player(true);
	gfx_->set_state(GraphicsState::FadeIn);
}

void RealPlayingState::reset() {
	delta_frame_ = {};
	loaded_rooms_.clear();
	text_->reset();
	anims_->reset();
	room_ = nullptr;
	undo_stack_->reset();
	objs_ = std::make_unique<GameObjectArray>();
}

void RealPlayingState::world_reset() {
	// Forget all the maps we know
	global_->add_flag(HUB_ACCESSED_GLOBAL_FLAGS[int(HubCode::Alpha)]);
	savefile_->make_emergency_save(this);
	reset();
	savefile_->world_reset();
	// Start anew in the world reset start room (not necessarily the same as the "new file start room"!)
	play_from_map(WORLD_RESET_START_MAP);
}