#include "stdafx.h"
#include "testplayingstate.h"

#include "room.h"
#include "roommap.h"
#include "player.h"
#include "car.h"

TestPlayingState::TestPlayingState(const std::string& map_name): PlayingState() {
    load_room(map_name, true);
	activate_room(map_name);
	snap_camera_to_player();
    room_->map()->set_initial_state(false);
}

TestPlayingState::~TestPlayingState() {}

bool TestPlayingState::load_room(const std::string& name, bool use_default_player) {
	std::filesystem::path path = (MAPS_TEMP / name).concat(".map");
	if (!std::filesystem::exists(path)) {
		path = (MAPS_MAIN / name).concat(".map");
		if (!std::filesystem::exists(path)) {
			return false;
		}
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