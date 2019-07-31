#include "stdafx.h"
#include "testplayingstate.h"

#include "room.h"
#include "roommap.h"
#include "player.h"
#include "car.h"

TestPlayingState::TestPlayingState(std::string map_name): PlayingState() {
    load_room(map_name, true);
	activate_room(map_name);
	snap_camera_to_player();
    room_->map()->set_initial_state(false);
}

TestPlayingState::~TestPlayingState() {}

bool TestPlayingState::load_room_from_temp(std::string name, bool use_default_player) {
	std::filesystem::path path = (MAPS_TEMP / name).concat(".map");
	if (!std::filesystem::exists(path)) {
		return false;
	}
	load_room_from_path(path, use_default_player);
	return true;
}

bool TestPlayingState::load_room(std::string name, bool use_default_player) {
	std::filesystem::path path = (MAPS_MAIN / name).concat(".map");
	if (!std::filesystem::exists(path)) {
		return false;
	}
	load_room_from_path(path, use_default_player);
	return true;
}