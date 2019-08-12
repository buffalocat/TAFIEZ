#include "stdafx.h"
#include "car.h"

#include "gameobject.h"
#include "roommap.h"
#include "player.h"
#include "texture_constants.h"
#include "mapfile.h"
#include "graphicsmanager.h"


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

bool Car::valid_parent(GameObject* obj) {
	return dynamic_cast<ColoredBlock*>(obj);
}

BlockTexture Car::texture() {
	return BlockTexture::Car;
}

void Car::collect_sticky_links(RoomMap* map, Sticky, std::vector<GameObject*>& to_check) {
    Player* player = dynamic_cast<Player*>(map->view(pos_above()));
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

void Car::draw(GraphicsManager* gfx, FPoint3 p) {
	if (int color = next_color()) {
		gfx->six_squares.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.01f, 1.01f, 1.01f), BlockTexture::Blank, color);
	}
}

std::unique_ptr<ObjectModifier> Car::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    auto dup = std::make_unique<Car>(*this);
    dup->parent_ = parent;
    return std::move(dup);
}
