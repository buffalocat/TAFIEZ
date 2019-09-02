#include "stdafx.h"
#include "objectmodifier.h"

#include "gameobject.h"
#include "texture_constants.h"

ObjectModifier::ObjectModifier(GameObject* parent): parent_ {parent} {}

ObjectModifier::~ObjectModifier() {}

void ObjectModifier::serialize(MapFileO&) {}

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

void ObjectModifier::draw(GraphicsManager* gfx, FPoint3 p) {}

void ObjectModifier::setup_on_put(RoomMap*, bool real) {}

void ObjectModifier::cleanup_on_take(RoomMap*, bool real) {}

void ObjectModifier::setup_on_editor_creation(EditorGlobalData* global, Room* room) {}

void ObjectModifier::cleanup_on_editor_destruction(EditorGlobalData* global) {}

void ObjectModifier::destroy(MoveProcessor*, CauseOfDeath, bool collect_links) {}

void ObjectModifier::undestroy() {}

void ObjectModifier::map_callback(RoomMap*, DeltaFrame*, MoveProcessor*) {}

void ObjectModifier::collect_special_links(RoomMap*, std::vector<GameObject*>&) {}
