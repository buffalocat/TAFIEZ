#include "stdafx.h"
#include "player.h"

#include "pushblock.h"
#include "snakeblock.h"

#include "delta.h"
#include "moveprocessor.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "texture_constants.h"

#include "mapfile.h"
#include "car.h"

Player::Player(Point3 pos, PlayerState state) : GameObject(pos, true, true), car_{}, state_{ state } {
	driven_ = true;
}

Player::~Player() {}

void Player::serialize(MapFileO& file) {
	file << state_;
}

std::unique_ptr<GameObject> Player::deserialize(MapFileI& file) {
	Point3 pos{ file.read_point3() };
	PlayerState state = static_cast<PlayerState>(file.read_byte());
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
	case PlayerState::Free:
		return BLUE;
		break;
	case PlayerState::Bound:
		return PINK;
		break;
	case PlayerState::RidingNormal:
		return gravitable_ ? RED : GREEN;
		break;
	case PlayerState::RidingHidden:
		return RED;
		break;
	default:
		return NO_COLOR;
		break;
	}
}

void Player::set_free(DeltaFrame* delta_frame) {
	if (delta_frame) {
		delta_frame->push(std::make_unique<PlayerStateDelta>(this));
	}
	state_ = PlayerState::Free;
	set_car(nullptr);
}

void Player::set_bound() {
	state_ = PlayerState::Bound;
	set_car(nullptr);
}

void Player::set_strictest(RoomMap* map, DeltaFrame* delta_frame) {
	if (delta_frame) {
		delta_frame->push(std::make_unique<PlayerStateDelta>(this));
	}
	if (auto* colored = dynamic_cast<ColoredBlock*>(map->view(shifted_pos({ 0,0,-1 })))) {
		if (auto* car = dynamic_cast<Car*>(colored->modifier())) {
			switch (car->type_) {
			case CarType::Normal:
			case CarType::Hover:
				state_ = PlayerState::RidingNormal;
				set_car(car);
				break;
			case CarType::Locked:
				set_bound();
				break;
			case CarType::Convertible:
				set_bound();
				break;
			}
		} else {
			set_bound();
		}
	} else {
		set_free(nullptr);
	}
}

// Make sure the player is allowed to be in the state it thinks it's in
void Player::validate_state(RoomMap* map, DeltaFrame* delta_frame) {
	switch (state_) {
	case PlayerState::RidingNormal:
		set_strictest(map, delta_frame);
		break;
	case PlayerState::Bound:
		if (!dynamic_cast<ColoredBlock*>(map->view(shifted_pos({ 0,0,-1 })))) {
			set_free(delta_frame);
		}
		break;
	default:
		break;
	}
}

void Player::set_car(Car* car) {
	if (car_) {
		car_->player_ = nullptr;
	}
	car_ = car;
	if (car_) {
		car_->player_ = this;
	}
}

bool Player::toggle_riding(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	auto delta = std::make_unique<PlayerStateDelta>(this);
	switch (state_) {
	case PlayerState::Free:
		return false;
	case PlayerState::Bound:
		if (auto* car = car_bound(map)) {
			switch (car->type_) {
			case CarType::Locked:
				return false;
			case CarType::Normal:
			case CarType::Hover:
				state_ = PlayerState::RidingNormal;
				set_car(car);
				break;
			case CarType::Convertible:
				if (auto* above = map->view(pos_ + Point3{ 0,0,1 })) {
					mp->add_to_fall_check(above);
				}
				map->take_from_map(this, true, true, delta_frame);
				state_ = PlayerState::RidingHidden;
				set_car(car);
				break;
			}
		} else {
			return false;
		}
		break;
	case PlayerState::RidingHidden:
		if (map->view(pos_)) {
			return false;
		} else {
			map->put_in_map(this, true, true, delta_frame);
		}
		// Fallthrough
	case PlayerState::RidingNormal:
		state_ = PlayerState::Bound;
		set_car(nullptr);
		break;
	}
	delta_frame->push(std::move(delta));
	return true;
}

void Player::destroy(MoveProcessor* mp, CauseOfDeath death, bool collect_links) {
	if (collect_links) {
		if (Car* car = car_riding()) {
			mp->fall_check_.push_back(car->parent_);
		}
	}
	mp->delta_frame_->push(std::make_unique<DestructionDelta>(this));
	mp->map_->player_cycle_->remove_player(this, mp->delta_frame_);
	death_ = death;
}

void Player::undestroy() {
	death_ = CauseOfDeath::None;
}

CauseOfDeath Player::death() {
	return death_;
}

Car* Player::car_riding() {
	switch (state_) {
	case PlayerState::Free:
	case PlayerState::Bound:
		return nullptr;
	case PlayerState::RidingNormal:
	case PlayerState::RidingHidden:
		return car_;
	default:
		return nullptr;
	}
}

Car* Player::car_bound(RoomMap* map) {
    switch (state_) {
	case PlayerState::Free:
		return nullptr;
	case PlayerState::Bound:
		return dynamic_cast<Car*>(map->view(shifted_pos({ 0,0,-1 }))->modifier());
	case PlayerState::RidingNormal:
	case PlayerState::RidingHidden:
		return car_;
	default:
		return nullptr;
    }
}

PlayerState Player::state() {
	return state_;
}

void Player::draw(GraphicsManager* gfx) {
    FPoint3 p = real_pos();
	switch (state_) {
	case PlayerState::RidingNormal:
	{
		auto* player_model = &gfx->cube;
		auto* windshield_model = &gfx->windshield;
		if (auto* snake = dynamic_cast<SnakeBlock*>(car_->parent_)) {
			player_model = &gfx->diamond;
			windshield_model = &gfx->windshield_diamond;
		}
		player_model->push_instance(glm::vec3(p.x, p.y, p.z - 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), BlockTexture::LightEdges, color());
		windshield_model->push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.0f, 1.0f, 1.0f), BlockTexture::Darker, car_->parent_->color());
		break;
	}
	case PlayerState::Bound:
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(0.6f, 0.6f, 0.6f), BlockTexture::LightEdges, color());
		break;
	case PlayerState::Free:
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(0.6f, 0.6f, 0.6f), BlockTexture::LightEdges, color());
		break;
	case PlayerState::RidingHidden:
	default:
		break;
	}
}

FPoint3 Player::cam_pos() {
	switch (state_) {
	case PlayerState::RidingNormal:
	case PlayerState::RidingHidden:
		return car_ ? (car_->parent_->real_pos() + Point3{ 0,0,1 }) : pos_;
	case PlayerState::Free:
	case PlayerState::Bound:
	default:
		return real_pos();
	}
}

void Player::collect_special_links(RoomMap* map, std::vector<GameObject*>& links) {
    if (state_ == PlayerState::RidingNormal) {
        links.push_back(map->view(shifted_pos({0,0,-1})));
    }
}

PlayerStateDelta::PlayerStateDelta(Player* player) :
	player_{ player }, car_{ player->car_ }, state_{ player->state_ }, death_{ player->death_ } {}

PlayerStateDelta::~PlayerStateDelta() {}

void PlayerStateDelta::revert() {
	player_->state_ = state_;
	player_->death_ = death_;
	player_->set_car(car_);
}