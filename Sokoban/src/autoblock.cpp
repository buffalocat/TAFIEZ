#include "stdafx.h"
#include "autoblock.h"

#include "gameobject.h"
#include "texture_constants.h"
#include "roommap.h"

AutoBlock::AutoBlock(GameObject* parent, RoomMap* room_map): ObjectModifier(parent), map_ {room_map} {}

AutoBlock::~AutoBlock() {}

std::string AutoBlock::name() {
    return "AutoBlock";
}

ModCode AutoBlock::mod_code() {
    return ModCode::AutoBlock;
}

void AutoBlock::serialize(MapFileO&) {}

void AutoBlock::deserialize(MapFileI& file, RoomMap* room_map, GameObject* parent) {
    parent->set_modifier(std::make_unique<AutoBlock>(parent, room_map));
}

bool AutoBlock::valid_parent(GameObject* obj) {
	return dynamic_cast<ColoredBlock*>(obj);
}

BlockTexture AutoBlock::texture() {
	return BlockTexture::AutoBlock;
}

void AutoBlock::setup_on_put(RoomMap* room_map) {
	room_map->autos_.push_back(this);
}

void AutoBlock::cleanup_on_take(RoomMap* room_map) {
	room_map->remove_auto(this);
}

std::unique_ptr<ObjectModifier> AutoBlock::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    return std::make_unique<AutoBlock>(parent, map_);
}
