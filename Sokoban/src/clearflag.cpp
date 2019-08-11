#include "stdafx.h"
#include "clearflag.h"

#include "gameobject.h"
#include "mapfile.h"

ClearFlag::ClearFlag(GameObject* parent, bool real, bool split) : ObjectModifier(parent),
real_{ real }, split_{ split } {}

ClearFlag::~ClearFlag() {}

std::string ClearFlag::name() {
	return "ClearFlag";
}

ModCode ClearFlag::mod_code() {
	return ModCode::ClearFlag;
}

void ClearFlag::serialize(MapFileO& file) {
	file << real_ << split_;
}

void ClearFlag::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	bool real, split;
	file >> real >> split;
	parent->set_modifier(std::make_unique<ClearFlag>(parent, real, split));
}

std::unique_ptr<ObjectModifier> ClearFlag::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<ClearFlag>(*this);
	dup->parent_ = parent;
	return std::move(dup);
}
