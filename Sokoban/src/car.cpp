#include "stdafx.h"
#include "car.h"

#include "gameobject.h"
#include "roommap.h"
#include "player.h"
#include "texture_constants.h"


Car::Car(GameObject* parent, ColorCycle color_cycle): ObjectModifier(parent), color_cycle_ {color_cycle} {}

Car::~Car() {}

std::string Car::name() {
    return "Car";
}

ModCode Car::mod_code() {
    return ModCode::Car;
}

void Car::serialize(MapFileO& file) {
    file << color_cycle_;
}

void Car::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
    ColorCycle color_cycle;
    file >> color_cycle;
    parent->set_modifier(std::make_unique<Car>(parent, color_cycle));
}

BlockTexture Car::texture() {
	return BlockTexture::Car;
}

void Car::collect_sticky_links(RoomMap* room_map, Sticky, std::vector<GameObject*>& to_check) {
    Player* player = dynamic_cast<Player*>(room_map->view(pos_above()));
    if (player) {
        to_check.push_back(player);
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

std::unique_ptr<ObjectModifier> Car::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    auto dup = std::make_unique<Car>(*this);
    dup->parent_ = parent;
    return std::move(dup);
}
