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
	PlayingState(parent, savefile->global_.get()), savefile_{ savefile } {
	savefile_->init_state(this);
}

RealPlayingState::RealPlayingState(RealPlayingState* rps) :
	PlayingState(rps->parent_.get(), rps->savefile_->global_.get()),
	savefile_{ rps->savefile_ } {
	savefile_->init_state(this);
}

RealPlayingState::~RealPlayingState() {
	if (room_) {
		make_subsave(SaveType::Emergency);
		savefile_->unload_state(this);
	}
}

void RealPlayingState::play_from_map(std::string starting_map) {
	load_room(starting_map, true);
	activate_room(starting_map);
	move_camera_to_player(true);
	gfx_->set_state(GraphicsState::FadeIn);
}

bool RealPlayingState::load_room(std::string name, bool use_default_player) {
	bool from_main;
	std::filesystem::path path = savefile_->get_path(this, name, &from_main);
	if (!std::filesystem::exists(path)) {
		return false;
	}
	load_room_from_path(path, use_default_player);
	return true;
}

void RealPlayingState::main_loop() {
	PlayingState::main_loop();
}

void RealPlayingState::make_subsave(SaveType type, unsigned int save_index, AutosavePanel* panel) {
	ensure_safe_delta_state();
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

bool RealPlayingState::load_subsave_dispatch(SaveType type, unsigned int index) {
	auto new_rps = std::make_unique<RealPlayingState>(this);
	if (savefile_->load_subsave_dispatch(type, index, new_rps.get())) {		
		new_rps->play_from_loaded_subsave();
		defer_to_sibling(std::move(new_rps));
		return true;
	}
	return false;
}

void RealPlayingState::play_from_loaded_subsave() {
	std::string cur_room_name = savefile_->cur_room_name_;
	load_room(cur_room_name, false);
	activate_room(cur_room_name);
	move_camera_to_player(true);
	gfx_->set_state(GraphicsState::FadeIn);
}

void RealPlayingState::world_reset() {
	// Forget all the maps we know
	global_->add_flag(HUB_ACCESSED_GLOBAL_FLAGS[int(HubCode::Alpha)]);
	auto new_rps = std::make_unique<RealPlayingState>(this);
	// Start anew in the world reset start room (not necessarily the same as the "new file start room"!)
	new_rps->play_from_map(WORLD_RESET_START_MAP);
	defer_to_sibling(std::move(new_rps));
}

void RealPlayingState::ensure_safe_delta_state() {
	if (delta_frame_) {
		delta_frame_->revert(this);
	}
	/*
	while (player_doa()->death_ != CauseOfDeath::None) {
		undo_stack_->pop();
	}*/
}
