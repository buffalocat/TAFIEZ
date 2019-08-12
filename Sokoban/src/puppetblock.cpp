#include "stdafx.h"
#include "puppetblock.h"

#include "gameobject.h"
#include "texture_constants.h"
#include "roommap.h"

PuppetBlock::PuppetBlock(GameObject* parent) : ObjectModifier(parent) {}

PuppetBlock::~PuppetBlock() {}

std::string PuppetBlock::name() {
	return "PuppetBlock";
}

ModCode PuppetBlock::mod_code() {
	return ModCode::PuppetBlock;
}

void PuppetBlock::serialize(MapFileO&) {}

void PuppetBlock::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	parent->set_modifier(std::make_unique<PuppetBlock>(parent));
}

bool PuppetBlock::valid_parent(GameObject* obj) {
	return dynamic_cast<ColoredBlock*>(obj);
}

BlockTexture PuppetBlock::texture() {
	return BlockTexture::PuppetBlock;
}

void PuppetBlock::setup_on_put(RoomMap* map, bool real) {
	if (real) {
		map->puppets_.push_back(this);
	}
}

void PuppetBlock::cleanup_on_take(RoomMap* map, bool real) {
	if (real) {
		map->remove_puppet(this);
	}
}

std::unique_ptr<ObjectModifier> PuppetBlock::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	return std::make_unique<PuppetBlock>(parent);
}
