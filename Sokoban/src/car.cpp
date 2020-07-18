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
	if (type_ == CarType::Convertible) {
		file << (player_ != nullptr);
	}
}

void Car::deserialize(MapFileI& file, RoomMap* map, GameObject* parent) {
	CarType type = static_cast<CarType>(file.read_byte());
    ColorCycle color_cycle;
    file >> color_cycle;
	auto car = std::make_unique<Car>(parent, type, color_cycle);
	if (type == CarType::Convertible) {
		if (file.read_byte()) {
			auto riding_player = std::make_unique<Player>(parent->shifted_pos({ 0,0,1 }), PlayerState::RidingHidden);
			riding_player->set_car(car.get());
			map->push_to_object_array(std::move(riding_player), nullptr);
		}
	}
	parent->set_modifier(std::move(car));
}

bool Car::valid_parent(GameObject* obj) {
	return dynamic_cast<ColoredBlock*>(obj);
}

GameObject* Car::get_subordinate_object() {
	if (player_ && !player_->tangible_) {
		return player_;
	}
	return nullptr;
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
	case CarType::Binding:
		return BlockTexture::BindingCar;
	default:
		return BlockTexture::Default;
	}
}

void Car::collect_special_links(std::vector<GameObject*>& to_check) {
	if (player_ && player_->tangible_) {
        to_check.push_back(player_);
    }
}

bool Car::is_multi_color() {
	return color_cycle_.size_ > 0;
}

void Car::cycle_color(bool undo) {
	if (auto* cb = dynamic_cast<ColoredBlock*>(parent_)) {
		color_cycle_.cycle(&cb->color_, undo);
	}
}

int Car::next_color() {
	return color_cycle_.next_color();
}

void Car::handle_movement(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
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
	map->moved_cars_.push_back(this);
}

void Car::cleanup_on_take(RoomMap* map, DeltaFrame*, bool real) {}

void Car::destroy(MoveProcessor* mp, CauseOfDeath death) {
	if (player_) {
		switch (type_) {
		case CarType::Normal:
		case CarType::Hover:
			if (!player_->gravitable_) {
				player_->gravitable_ = true;
				mp->delta_frame_->push(std::make_unique<ToggleGravitableDelta>(player_));
			}
			player_->set_free(mp->delta_frame_);
			break;
		case CarType::Convertible:
			player_->destroy(mp, death);
			break;
		}
	}
}

bool Car::update_animation(PlayingState*) {
	if (animation_time_ > 0) {
		--animation_time_;
		return false;
	} else {
		reset_animation();
		return true;
	}
}

void Car::reset_animation() {
	animation_state_ = CarAnimationState::None;
	if (animation_player_) {
		animation_player_->animation_car_ = nullptr;
		animation_player_ = nullptr;
	}
	animation_time_ = 0;
}

const float WINDSHIELD_HEIGHTS[MAX_CAR_ANIMATION_FRAMES] = { 0.1f, 0.3f, 0.7f, 0.9f };

void Car::draw(GraphicsManager* gfx, FPoint3 p) {
	if (int color = next_color()) {
		ModelInstancer& squares_model = parent_->is_snake() ? gfx->six_squares_diamond : gfx->six_squares;
		squares_model.push_instance(glm::vec3(p), glm::vec3(1.01f), BlockTexture::Blank, color);
	}
	switch (type_) {
	case CarType::Normal:
	case CarType::Hover:
	{
		ModelInstancer& windshield_model = parent_->is_snake() ? gfx->windshield_diamond : gfx->windshield;
		float windshield_height;
		switch (animation_state_) {
		case CarAnimationState::None:
			windshield_height = player_ ? 1.0f : 0.0f;
			break;
		case CarAnimationState::Riding:
			windshield_height = 1.0f - WINDSHIELD_HEIGHTS[animation_time_];
			break;
		case CarAnimationState::Unriding:
			windshield_height = WINDSHIELD_HEIGHTS[animation_time_];
			break;
		}
		if (windshield_height > 0) {
			windshield_model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5 + windshield_height / 2.0), glm::vec3(1.0f, 1.0f, windshield_height), BlockTexture::Darker, parent_->color());
		}
		break;
	}
	case CarType::Convertible:
	{
		Player* player = animation_player_ ? animation_player_ : player_;
		if (player) {
			player->draw(gfx);
		}
		break;
	}
	default:
		break;
	}
}

void Car::draw_squished(GraphicsManager* gfx, FPoint3 p, float scale) {
	if (int color = next_color()) {
		ModelInstancer& squares_model = parent_->is_snake() ? gfx->six_squares_diamond : gfx->six_squares;
		squares_model.push_instance(glm::vec3(p), glm::vec3(scale, scale, 1.01f), BlockTexture::Blank, color);
	}
	switch (type_) {
	case CarType::Normal:
	case CarType::Hover:
	{
		ModelInstancer& windshield_model = parent_->is_snake() ? gfx->windshield_diamond : gfx->windshield;
		float windshield_height;
		switch (animation_state_) {
		case CarAnimationState::None:
			windshield_height = player_ ? 1.0f : 0.0f;
			break;
		case CarAnimationState::Riding:
			windshield_height = 1.0f - WINDSHIELD_HEIGHTS[animation_time_];
			break;
		case CarAnimationState::Unriding:
			windshield_height = WINDSHIELD_HEIGHTS[animation_time_];
			break;
		}
		if (windshield_height > 0) {
			windshield_model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5 + windshield_height / 2.0), glm::vec3(scale, scale, windshield_height), BlockTexture::Darker, parent_->color());
		}
		break;
	}
	case CarType::Convertible:
	{
		Player* player = animation_player_ ? animation_player_ : player_;
		if (player) {
			player->draw_squished(gfx, p, scale);
		}
		break;
	}
	default:
		break;
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
