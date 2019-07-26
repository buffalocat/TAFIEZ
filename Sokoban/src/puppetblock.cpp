#include "stdafx.h"
#include "puppetblock.h"

#include "gameobject.h"
#include "texture_constants.h"
#include "roommap.h"

PuppetBlock::PuppetBlock(GameObject* parent, RoomMap* room_map) : ObjectModifier(parent), map_{ room_map } {}

PuppetBlock::~PuppetBlock() {}

std::string PuppetBlock::name() {
	return "PuppetBlock";
}

ModCode PuppetBlock::mod_code() {
	return ModCode::PuppetBlock;
}

void PuppetBlock::serialize(MapFileO&) {}

void PuppetBlock::deserialize(MapFileI& file, RoomMap* room_map, GameObject* parent) {
	parent->set_modifier(std::make_unique<PuppetBlock>(parent, room_map));
}

bool PuppetBlock::valid_parent(GameObject* obj) {
	return dynamic_cast<ColoredBlock*>(obj);
}

BlockTexture PuppetBlock::texture() {
	return BlockTexture::PuppetBlock;
}

void PuppetBlock::setup_on_put(RoomMap* room_map) {
	room_map->puppets_.push_back(this);
}

void PuppetBlock::cleanup_on_take(RoomMap* room_map) {
	room_map->remove_puppet(this);
}

std::unique_ptr<ObjectModifier> PuppetBlock::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	return std::make_unique<PuppetBlock>(parent, map_);
}
