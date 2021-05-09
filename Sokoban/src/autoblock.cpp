#include "stdafx.h"
#include "autoblock.h"

#include "gameobject.h"
#include "texture_constants.h"
#include "roommap.h"

AutoBlock::AutoBlock(GameObject* parent): ObjectModifier(parent) {}

AutoBlock::~AutoBlock() {}

void AutoBlock::make_str(std::string& str) {
	str += "AutoBlock";
}

ModCode AutoBlock::mod_code() {
    return ModCode::AutoBlock;
}

void AutoBlock::deserialize(MapFileI&, GameObjectArray*, GameObject* parent) {
    parent->set_modifier(std::make_unique<AutoBlock>(parent));
}

bool AutoBlock::valid_parent(GameObject* obj) {
	return dynamic_cast<ColoredBlock*>(obj);
}

BlockTexture AutoBlock::texture() {
	return BlockTexture::AutoBlock;
}

void AutoBlock::setup_on_put(RoomMap* map, DeltaFrame*, bool real) {
	if (real) {
		map->autos_.push_back(this);
	}
}

void AutoBlock::cleanup_on_take(RoomMap* map, DeltaFrame*, bool real) {
	if (real) {
		map->remove_auto(this);
	}
}

std::unique_ptr<ObjectModifier> AutoBlock::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    return std::make_unique<AutoBlock>(parent);
}
