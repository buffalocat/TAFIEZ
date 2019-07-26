#include "stdafx.h"
#include "objectmodifier.h"

#include "gameobject.h"
#include "texture_constants.h"

ObjectModifier::ObjectModifier(GameObject* parent): parent_ {parent} {}

ObjectModifier::~ObjectModifier() {}

bool ObjectModifier::relation_check() {
    return false;
}

void ObjectModifier::relation_serialize(MapFileO&) {}

// The "default" case is "Block"
bool ObjectModifier::valid_parent(GameObject* obj) {
	return dynamic_cast<Block*>(obj);
}


Point3 ObjectModifier::pos() {
    return parent_->pos_;
}

Point3 ObjectModifier::shifted_pos(Point3 d) {
    return parent_->pos_ + d;
}

void ObjectModifier::shift_internal_pos(Point3 d) {}

Point3 ObjectModifier::pos_above() {
    return parent_->pos_ + Point3{0,0,1};
}

bool ObjectModifier::pushable() {
    return parent_->pushable_;
}

bool ObjectModifier::gravitable() {
    return parent_->gravitable_;
}

BlockTexture ObjectModifier::texture() {
	return BlockTexture::Default;
}

void ObjectModifier::setup_on_put(RoomMap* room_map) {}

void ObjectModifier::cleanup_on_take(RoomMap* room_map) {}

void ObjectModifier::cleanup_on_destruction(RoomMap* room_map) {}

void ObjectModifier::setup_on_undestruction(RoomMap* room_map) {}

void ObjectModifier::map_callback(RoomMap*, DeltaFrame*, MoveProcessor*) {}

void ObjectModifier::collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&) {}
