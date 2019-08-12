#include "stdafx.h"
#include "realplayingstate.h"

#include "common_constants.h"
#include "room.h"
#include "roommap.h"
#include "savefile.h"
#include "mapfile.h"

RealPlayingState::RealPlayingState(std::unique_ptr<SaveFile> save) :
	PlayingState(), savefile_{ std::move(save) } {
	save->global_ = global_.get();
}

RealPlayingState::~RealPlayingState() {}

void RealPlayingState::start_from_map(std::string starting_map) {
	load_room(starting_map, true);
	activate_room(starting_map);
	snap_camera_to_player();
	room_->map()->set_initial_state(false);
}

bool RealPlayingState::load_room(std::string name, bool use_default_player) {
	bool from_main;
	std::filesystem::path path = savefile_->get_path(name, &from_main);
	if (!std::filesystem::exists(path)) {
		return false;
	}
	load_room_from_path(path, use_default_player || !from_main);
	return true;
}

void RealPlayingState::make_subsave() {
	savefile_->make_subsave(loaded_rooms_, active_room()->name());
}

void RealPlayingState::load_most_recent_subsave() {
	std::string cur_room_name{};
	savefile_->load_most_recent_subsave(&cur_room_name);
	load_room(cur_room_name, false);
	activate_room(cur_room_name);
}