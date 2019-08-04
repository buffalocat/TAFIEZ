#include "stdafx.h"
#include "player.h"

#include "pushblock.h"

#include "delta.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "texture_constants.h"

#include "mapfile.h"
#include "car.h"

Player::Player(Point3 pos, RidingState state): GameObject(pos, true, true), state_ {state} {
	driven_ = true;
}

Player::~Player() {}

void Player::serialize(MapFileO& file) {
	file << state_;
}

std::unique_ptr<GameObject> Player::deserialize(MapFileI& file) {
	Point3 pos{ file.read_point3() };
	RidingState state = static_cast<RidingState>(file.read_byte());
	return std::make_unique<Player>(pos, state);
}

std::string Player::name() {
    return "Player";
}

ObjCode Player::obj_code() {
    return ObjCode::Player;
}

int Player::color() {
	switch (state_) {
	case RidingState::Free:
		return BLUE;
		break;
	case RidingState::Bound:
		return PINK;
		break;
	case RidingState::Riding:
		return RED;
		break;
	default:
		return NO_COLOR;
		break;
	}
}

void Player::toggle_riding(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (state_ == RidingState::Riding) {
        delta_frame->push(std::make_unique<RidingStateDelta>(this, state_));
        state_ = RidingState::Bound;
    } else if (state_ == RidingState::Bound) {
        if (dynamic_cast<Car*>(room_map->view(shifted_pos({0,0,-1}))->modifier())) {
            delta_frame->push(std::make_unique<RidingStateDelta>(this, state_));
            state_ = RidingState::Riding;
        }
    }
}

Car* Player::get_car(RoomMap* room_map, bool strict) {
    if (state_ == RidingState::Free || (strict && state_ == RidingState::Bound)) {
        return nullptr;
    } else {
        //NOTE: if there are no bugs elsewhere, this could be a static cast
        return dynamic_cast<Car*>(room_map->view(shifted_pos({0,0,-1}))->modifier());
    }
}

void Player::draw(GraphicsManager* gfx) {
    FPoint3 p = real_pos();
	switch (state_) {
	case RidingState::Riding:
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z - 0.7f), glm::vec3(0.5f, 0.5f, 0.5f), BlockTexture::Blank, color());
		gfx->windshield.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.0f, 1.0f, 1.0f), BlockTexture::Blank, GREY);
		break;
	case RidingState::Bound:
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(0.6f, 0.6f, 0.6f), BlockTexture::Blank, color());
		break;
	case RidingState::Free:
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(0.6f, 0.6f, 0.6f), BlockTexture::Blank, color());
		break;
	}
	
	// TODO: make it so the player *can't* have a modifier
    if (modifier_) {
        modifier()->draw(gfx, p);
    }
	if (state_ == RidingState::Riding) {
		
	}
}

FPoint3 Player::cam_pos() {
	if (state_ == RidingState::Riding) {
		return real_pos() + FPoint3{ 0, 0, -2 };
	} else {
		return real_pos() + FPoint3{ 0, 0, -1 };
	}
}

// NOTE: if the Player becomes a subclass of a more general "Passenger" type, move this up to that class.
void Player::collect_special_links(RoomMap* room_map, Sticky sticky_level, std::vector<GameObject*>& links) {
    if (state_ == RidingState::Riding) {
        links.push_back(room_map->view(shifted_pos({0,0,-1})));
    }
}
