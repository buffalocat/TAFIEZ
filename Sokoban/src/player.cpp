#include "stdafx.h"
#include "player.h"

#include "pushblock.h"
#include "snakeblock.h"

#include "delta.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "texture_constants.h"

#include "mapfile.h"
#include "car.h"

Player::Player(Point3 pos, RidingState state) : GameObject(pos, true, true), car_{}, state_{ state } {
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

bool Player::bound() {
	return state_ == RidingState::Bound;
}

void Player::set_free() {
	state_ = RidingState::Free;
	car_ = nullptr;
}

void Player::set_strictest(RoomMap* room_map) {
	if (auto* colored = dynamic_cast<ColoredBlock*>(room_map->view(shifted_pos({ 0,0,-1 })))) {
		if (auto* car = dynamic_cast<Car*>(colored->modifier())) {
			set_riding(car);
		} else {
			set_bound();
		}
	} else {
		set_free();
	}
}

void Player::validate_state(RoomMap* room_map) {
	switch (state_) {
	case RidingState::Riding:
		if (auto* colored = dynamic_cast<ColoredBlock*>(room_map->view(shifted_pos({ 0,0,-1 })))) {
			if (auto* car = dynamic_cast<Car*>(colored->modifier())) {
				car_ = car;
			} else {
				set_bound();
			}
		} else {
			set_free();
		}
		break;
	case RidingState::Bound:
		if (!dynamic_cast<ColoredBlock*>(room_map->view(shifted_pos({ 0,0,-1 })))) {
			set_free();
		}
		break;
	}
}


void Player::set_bound() {
	state_ = RidingState::Bound;
	car_ = nullptr;
}

void Player::set_riding(Car* car) {
	state_ = RidingState::Riding;
	car_ = car;
}

void Player::toggle_riding(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (state_ == RidingState::Riding) {
		if (delta_frame) {
			delta_frame->push(std::make_unique<RidingStateDelta>(this, car_, state_));
		}
		set_bound();
    } else if (state_ == RidingState::Bound) {
        if (auto* car = dynamic_cast<Car*>(room_map->view(shifted_pos({0,0,-1}))->modifier())) {
			if (delta_frame) {
				delta_frame->push(std::make_unique<RidingStateDelta>(this, nullptr, state_));
			}
			set_riding(car);
        }
    }
}

Car* Player::car_riding() {
	return car_;
}

Car* Player::car_bound(RoomMap* room_map) {
    if (state_ == RidingState::Free) {
		return nullptr;
    } else {
        return dynamic_cast<Car*>(room_map->view(shifted_pos({0,0,-1}))->modifier());
    }
}

void Player::draw(GraphicsManager* gfx) {
    FPoint3 p = real_pos();
	switch (state_) {
	case RidingState::Riding:
	{
		auto* player_model = &gfx->cube;
		auto* windshield_model = &gfx->windshield;
		if (auto* snake = dynamic_cast<SnakeBlock*>(car_->parent_)) {
			player_model = &gfx->diamond;
			windshield_model = &gfx->windshield_diamond;
		}
		player_model->push_instance(glm::vec3(p.x, p.y, p.z - 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), BlockTexture::Blank, color());
		windshield_model->push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.0f, 1.0f, 1.0f), BlockTexture::Darker, car_->parent_->color());
		break;
	}
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
