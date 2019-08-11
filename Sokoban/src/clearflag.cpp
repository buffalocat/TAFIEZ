#include "stdafx.h"
#include "clearflag.h"

#include "gameobject.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "texture_constants.h"
#include "roommap.h"

ClearFlag::ClearFlag(GameObject* parent, int count, bool real, char zone) :
	ObjectModifier(parent),
	count_{ count }, real_{ real }, zone_{ zone } {}

ClearFlag::~ClearFlag() {}

std::string ClearFlag::name() {
	return "ClearFlag";
}

ModCode ClearFlag::mod_code() {
	return ModCode::ClearFlag;
}

void ClearFlag::serialize(MapFileO& file) {
	file << real_;
}

void ClearFlag::deserialize(MapFileI& file, RoomMap* room_map, GameObject* parent) {
	bool real;
	file >> real;
	parent->set_modifier(std::make_unique<ClearFlag>(parent, real, room_map->clear_flag_req_, room_map->zone_));
}

std::unique_ptr<ObjectModifier> ClearFlag::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<ClearFlag>(*this);
	dup->parent_ = parent;
	return std::move(dup);
}

void ClearFlag::draw(GraphicsManager* gfx, FPoint3 p) {
	gfx->flag.push_instance(glm::vec3(p.x, p.y, p.z + 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), BlockTexture::Blank, GOLD);
}