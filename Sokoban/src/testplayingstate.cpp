#include "stdafx.h"
#include "testplayingstate.h"

#include "graphicsmanager.h"
#include "room.h"
#include "roommap.h"
#include "player.h"
#include "car.h"
#include "savefile.h"

TestPlayingState::TestPlayingState(GameState* parent, std::unique_ptr<PlayingGlobalData> global):
	PlayingState(parent, global.get()), global_unique_{ std::move(global) } {}

TestPlayingState::~TestPlayingState() {}

void TestPlayingState::init(std::string map_name) {
	load_room_from_temp(map_name, true);
	activate_room(map_name);
	move_camera_to_player(true);
}

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