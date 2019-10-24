#include "stdafx.h"
#include "worldresetkey.h"

#include "common_constants.h"
#include "graphicsmanager.h"
#include "gameobject.h"
#include "texture_constants.h"
#include "delta.h"
#include "player.h"
#include "roommap.h"
#include "savefile.h"

WorldResetKey::WorldResetKey(GameObject* parent, bool collected) :
	ObjectModifier(parent), collected_{ collected } {}

WorldResetKey::~WorldResetKey() {}

void WorldResetKey::deserialize(MapFileI& file, RoomMap* map, GameObject* parent) {
	bool collected = false;
	if (auto* global = map->global_) {
		collected = global->has_flag(WORLD_RESET_GLOBAL_ID);
	}
	parent->set_modifier(std::make_unique<WorldResetKey>(parent, collected));
}

void WorldResetKey::make_str(std::string& str) {
	str += "WorldResetKey";
}

ModCode WorldResetKey::mod_code() {
	return ModCode::WorldResetKey;
}

std::unique_ptr<ObjectModifier> WorldResetKey::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	return std::make_unique<WorldResetKey>(parent);
}

void WorldResetKey::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor*) {
	if (collected_) {
		return;
	}
	if (parent_->tangible_) {
		if (auto* above = map->view(pos_above())) {
			if (dynamic_cast<Player*>(above)) {
				collected_ = true;
			} else if (auto* player = dynamic_cast<Player*>(map->view(pos() + Point3{ 0, 0, 2 }))) {
				if (player->car_riding()) {
					collected_ = true;
				}
			}
		}
		if (collected_) {
			map->global_->add_flag(WORLD_RESET_GLOBAL_ID);
			delta_frame->push(std::make_unique<KeyCollectDelta>(this));
			delta_frame->push(std::make_unique<GlobalFlagDelta>(map->global_, WORLD_RESET_GLOBAL_ID));
		}
	}
}

void WorldResetKey::setup_on_put(RoomMap* map, DeltaFrame*, bool real) {
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
}

void WorldResetKey::cleanup_on_take(RoomMap* map, DeltaFrame*, bool real) {
	map->remove_listener(this, pos_above());
}

void WorldResetKey::draw(GraphicsManager* gfx, FPoint3 p) {
	if (!collected_) {
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z + 1), glm::vec3(0.2, 0.2, 0.2), BlockTexture::BrokenEdges, glm::vec4(0.5, 1.0, 0.7, 1.0));
	}
}


KeyCollectDelta::KeyCollectDelta(WorldResetKey* key) :
	Delta(), key_{ key } {}

KeyCollectDelta::~KeyCollectDelta() {}

void KeyCollectDelta::revert() {
	key_->collected_ = false;
}