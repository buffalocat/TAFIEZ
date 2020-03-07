#include "stdafx.h"
#include "objectmodifier.h"

#include "gameobject.h"
#include "texture_constants.h"
#include "savefile.h"

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

GameObject* ObjectModifier::get_subordinate_object() {
	return nullptr;
}

Point3 ObjectModifier::pos() {
    return parent_->pos_;
}

Point3 ObjectModifier::shifted_pos(Point3 d) {
    return parent_->pos_ + d;
}

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

void ObjectModifier::setup_on_put(RoomMap*, DeltaFrame*, bool real) {}

void ObjectModifier::cleanup_on_take(RoomMap*, DeltaFrame*, bool real) {}

void ObjectModifier::setup_on_editor_creation(EditorGlobalData* global, Room* room) {}

void ObjectModifier::cleanup_on_editor_destruction(EditorGlobalData* global) {}

void ObjectModifier::destroy(MoveProcessor*, CauseOfDeath) {}

void ObjectModifier::undestroy() {}

void ObjectModifier::signal_animation(AnimationManager*, DeltaFrame*) {}

bool ObjectModifier::update_animation(PlayingState*) {
	return true;
}

void ObjectModifier::reset_animation() {}

void ObjectModifier::map_callback(RoomMap*, DeltaFrame*, MoveProcessor*) {}

void ObjectModifier::collect_special_links(std::vector<GameObject*>&) {}


ModDestructionDelta::ModDestructionDelta(ObjectModifier* mod) :
	mod_{ mod } {}

ModDestructionDelta::~ModDestructionDelta() {}

void ModDestructionDelta::revert() {
	mod_->undestroy();
}


GlobalFlagDelta::GlobalFlagDelta(PlayingGlobalData* global, unsigned int flag) :
	Delta(), global_{ global }, flag_{ flag } {}

GlobalFlagDelta::~GlobalFlagDelta() {}

void GlobalFlagDelta::revert() {
	global_->remove_flag(flag_);
}