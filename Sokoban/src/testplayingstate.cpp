#include "stdafx.h"
#include "testplayingstate.h"

#include "room.h"
#include "roommap.h"
#include "player.h"
#include "car.h"

TestPlayingState::TestPlayingState(const std::string& name, Point3 pos): PlayingState() {
    activate_room(name);
	init_player(pos);
	snap_camera_to_player();
    room_->map()->set_initial_state(false);
}

TestPlayingState::~TestPlayingState() {}
