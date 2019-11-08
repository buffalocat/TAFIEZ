#include "stdafx.h"
#include "car.h"

#include "gameobject.h"
#include "roommap.h"
#include "player.h"
#include "texture_constants.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "moveprocessor.h"


Car::Car(GameObject* parent, CarType type, ColorCycle color_cycle) : ObjectModifier(parent), type_{ type }, color_cycle_{ color_cycle } {}

Car::~Car() {}

void Car::make_str(std::string& str) {
	str += "Car";
}

ModCode Car::mod_code() {
    return ModCode::Car;
}

void Car::serialize(MapFileO& file) {
    file << type_ << color_cycle_;
}

void Car::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	CarType type = static_cast<CarType>(file.read_byte());
    ColorCycle color_cycle;
    file >> color_cycle;
    parent->set_modifier(std::make_unique<Car>(parent, type, color_cycle));
}

bool Car::valid_parent(GameObject* obj) {
	return dynamic_cast<ColoredBlock*>(obj);
}

BlockTexture Car::texture() {
	switch (type_) {
	case CarType::Locked:
		return BlockTexture::LockedCar;
	case CarType::Normal:
		return BlockTexture::NormalCar;
	case CarType::Convertible:
		return BlockTexture::ConvertibleCar;
	case CarType::Hover:
		return BlockTexture::HoverCar;
	default:
		return BlockTexture::Default;
	}
}

void Car::shift_internal_pos(Point3 d) {
	if (player_ && !player_->tangible_) {
		player_->shift_internal_pos(d);
	}
}

void Car::collect_special_links(std::vector<GameObject*>& to_check) {
	if (player_ && player_->tangible_) {
        to_check.push_back(player_);
    }
}

bool Car::cycle_color(bool undo) {
	if (auto* cb = dynamic_cast<ColoredBlock*>(parent_)) {
		return color_cycle_.cycle(&cb->color_, undo);
	}
	return false;
}

int Car::next_color() {
	return color_cycle_.next_color();
}

void Car::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	// TODO: figure out a way to make this code run faster (on the same frame as input)
	if (player_) {
		switch (type_) {
		case CarType::Normal:
		case CarType::Hover:
			if (!(player_->pos_ == pos_above())) {
				mp->add_to_fall_check(player_);
				player_->set_strictest(map, delta_frame);
			}
			break;
		case CarType::Convertible:
			player_->abstract_put(pos_above(), delta_frame);
			break;
		}
	}
}

void Car::setup_on_put(RoomMap* map, DeltaFrame*, bool real) {
	map->activate_listener_of(this);
}

void Car::cleanup_on_take(RoomMap* map, DeltaFrame*, bool real) {}

void Car::destroy(MoveProcessor* mp, CauseOfDeath death) {
	if (player_) {
		switch (type_) {
		case CarType::Normal:
		case CarType::Hover:
			player_->set_free(mp->delta_frame_);
			break;
		case CarType::Convertible:
			player_->destroy(mp, death);
			break;
		}
	}
}

void Car::draw(GraphicsManager* gfx, FPoint3 p) {
	// TODO: Add rotated six_squares!!!
	if (int color = next_color()) {
		ModelInstancer& model = parent_->is_snake() ? gfx->six_squares_diamond : gfx->six_squares;
		model.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.01f, 1.01f, 1.01f), BlockTexture::Blank, color);
	}
}

std::unique_ptr<ObjectModifier> Car::duplicate(GameObject* parent, RoomMap* map, DeltaFrame* delta_frame) {
    auto dup = std::make_unique<Car>(*this);
    dup->parent_ = parent;
	switch (type_) {
	case CarType::Normal:
	case CarType::Hover:
		dup->player_ = nullptr;
		break;
	case CarType::Convertible:
	{
		if (player_) {
			auto player_dup = std::make_unique<Player>(pos_above(), PlayerState::RidingHidden);
			player_dup->set_car(dup.get());
			map->player_cycle_->add_player(player_dup.get(), delta_frame, false);
			map->push_to_object_array(std::move(player_dup), delta_frame);
		} else {
			dup->player_ = nullptr;
		}
		break;
	}
	}
    return std::move(dup);
}
