#include "stdafx.h"
#include "realplayingstate.h"

#include "room.h"
#include "roommap.h"

RealPlayingState::RealPlayingState(const std::string& savefile_dir) : PlayingState(), savefile_dir_{ savefile_dir } {
	activate_room("story1");
	init_player(Point3{ 2,1,2 });
	snap_camera_to_player();
	room_->map()->set_initial_state(false);
}

RealPlayingState::~RealPlayingState() {}
