#include "stdafx.h"
#include "realplayingstate.h"

#include "common_constants.h"
#include "room.h"
#include "roommap.h"
#include "savefile.h"
#include "mapfile.h"

RealPlayingState::RealPlayingState(std::unique_ptr<SaveFile> save) :
	PlayingState(), savefile_{ std::move(save) } {}

RealPlayingState::~RealPlayingState() {}

void RealPlayingState::start_from_map(const std::string& starting_map) {
	load_room(starting_map, true);
	activate_room(starting_map);
	snap_camera_to_player();
	room_->map()->set_initial_state(false);
}

bool RealPlayingState::load_room(std::string const& name, bool use_default_player) {
	bool from_main;
	std::filesystem::path path = savefile_->get_path(name, &from_main);
	if (!std::filesystem::exists(path)) {
		return false;
	}
	MapFileI file{ path };
	auto room = std::make_unique<Room>(name);
	if (use_default_player || !from_main) {
		room->load_from_file(*objs_, file, &player_);
	} else {
		room->load_from_file(*objs_, file, nullptr);
	}
	room->map()->set_initial_state(false);
	loaded_rooms_[name] = std::make_unique<PlayingRoom>(std::move(room));
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