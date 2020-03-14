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

RealPlayingState::RealPlayingState(SaveFile* savefile, GameState* parent) :
	PlayingState(parent, savefile->global_.get()), savefile_{ savefile } {}

RealPlayingState::~RealPlayingState() {
	if (delta_frame_) {
		delta_frame_->revert();
	}
	make_subsave();
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

void RealPlayingState::make_subsave() {
	savefile_->make_subsave(loaded_rooms_, active_room()->name());
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
	delta_frame_ = {};
	loaded_rooms_.clear();
	text_->reset();
	anims_->reset();
	room_ = nullptr;
	savefile_->world_reset();
	undo_stack_->reset();
	objs_ = std::make_unique<GameObjectArray>();
	// Start anew in the world reset start room (not necessarily the same as the "new file start room"!)
	play_from_map(WORLD_RESET_START_MAP);
}