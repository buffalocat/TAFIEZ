#include "stdafx.h"
#include "testplayingstate.h"

#include "room.h"
#include "roommap.h"
#include "player.h"
#include "car.h"

TestPlayingState::TestPlayingState(const std::string& name, Point3 pos): PlayingState() {
    activate_room(name);
    init_player(pos);
    room_->map()->set_initial_state(false);
}

TestPlayingState::~TestPlayingState() {}

void TestPlayingState::init_player(Point3 pos) {
    RidingState rs;
    // TODO: fix this hack
    GameObject* below = room_->map()->view({pos.x, pos.y, pos.z - 1});
    if (below) {
        if (dynamic_cast<Car*>(below->modifier())) {
            rs = RidingState::Riding;
        } else {
            rs = RidingState::Bound;
        }
    } else {
        rs = RidingState::Free;
    }
    auto player = std::make_unique<Player>(pos, rs);
    player_ = player.get();
    room_->map()->create(std::move(player), nullptr);
}
