#include "stdafx.h"
#include "realplayingstate.h"

#include "common_constants.h"
#include "room.h"
#include "roommap.h"
#include "savefile.h"
#include "mapfile.h"

RealPlayingState::RealPlayingState(const std::string& savefile_dir, const std::string& starting_map) :
	PlayingState(), savefile_{ std::make_unique<SaveFile>(savefile_dir) } {
	load_room(starting_map, true);
	activate_room(starting_map);
	snap_camera_to_player();
	room_->map()->set_initial_state(false);
}

RealPlayingState::~RealPlayingState() {}

bool RealPlayingState::load_room(const std::string& name, bool use_default_player) {
	std::filesystem::path path = savefile_->get_path(name);
	if (!std::filesystem::exists(path)) {
		return false;
	}
	MapFileI file{ path };
	auto room = std::make_unique<Room>(name);
	if (use_default_player) {
		room->load_from_file(*objs_, file, &player_);
	} else {
		room->load_from_file(*objs_, file, nullptr);
	}
	// Load dynamic component!
	room->map()->set_initial_state(false);
	loaded_rooms_[name] = std::make_unique<PlayingRoom>(std::move(room));
	return true;
}