#include "stdafx.h"
#include "gameobject.h"

#include "common_constants.h"

#include "roommap.h"
#include "graphicsmanager.h"
#include "texture_constants.h"
#include "delta.h"
#include "mapfile.h"
#include "objectmodifier.h"
#include "moveprocessor.h"

#include "component.h"


Sticky operator &(Sticky a, Sticky b) {
	return static_cast<Sticky>(static_cast<unsigned char>(a) &
		static_cast<unsigned char>(b));
}


// id_ begins in an "inconsistent" state - it *must* be set by the GameObjectArray
GameObject::GameObject(Point3 pos, bool pushable, bool gravitable) :
	pos_{ pos }, pushable_{ pushable }, gravitable_{ gravitable } {}

GameObject::~GameObject() {}

// Copy Constructor creates trivial unique_ptr members
GameObject::GameObject(const GameObject& obj) :
	pos_{ obj.pos_ }, pushable_{ obj.pushable_ }, gravitable_{ obj.gravitable_ } {}

std::string GameObject::to_str() {
	std::string mod_str{ "" };
	if (modifier_) {
		mod_str = "-";
		modifier_->make_str(mod_str);
	}
	char buf[256] = "";
	// TODO: Make this less ugly
	// This is only used to make the editor more user friendly, so it's not a huge deal.
	snprintf(buf, 256, "%s:%s:%s%s", name().c_str(), pushable_ ? "P" : "NP", gravitable_ ? "G" : "NG", mod_str.c_str());
	return std::string{ buf };
}

bool GameObject::relation_check() {
	return false;
}

bool GameObject::skip_serialization() {
	return false;
}

void GameObject::serialize(MapFileO& file) {}

void GameObject::relation_serialize(MapFileO& file) {}

Point3 GameObject::shifted_pos(Point3 d) {
	return pos_ + d;
}

void GameObject::shift_internal_pos(Point3 d) {
	pos_ += d;
	if (modifier_) {
		modifier_->shift_internal_pos(d);
	}
}

void GameObject::setup_on_put(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	if (modifier_) {
		modifier_->setup_on_put(map, delta_frame, real);
	}
}

void GameObject::cleanup_on_take(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	if (modifier_) {
		modifier_->cleanup_on_take(map, delta_frame, real);
	}
}

void GameObject::destroy(MoveProcessor* mp, CauseOfDeath death) {
	if (modifier_) {
		modifier_->destroy(mp, death);
	}
}

void GameObject::undestroy() {
	if (modifier_) {
		modifier_->undestroy();
	}
}

void GameObject::set_modifier(std::unique_ptr<ObjectModifier> mod) {
	modifier_ = std::move(mod);
}

ObjectModifier* GameObject::modifier() {
	return modifier_.get();
}

void GameObject::abstract_shift(Point3 dpos, DeltaFrame* delta_frame) {
	if (!(dpos == Point3{})) {
		pos_ += dpos;
		delta_frame->push(std::make_unique<AbstractShiftDelta>(this, dpos));
	}
}

void GameObject::abstract_put(Point3 pos, DeltaFrame* delta_frame) {
	delta_frame->push(std::make_unique<AbstractPutDelta>(this, pos_));
	pos_ = pos;
}

// NOTE: these can be static_casts as long as the code using them is careful
PushComponent* GameObject::push_comp() {
	return dynamic_cast<PushComponent*>(comp_);
}

FallComponent* GameObject::fall_comp() {
	return dynamic_cast<FallComponent*>(comp_);
}

void GameObject::collect_sticky_component(RoomMap* map, Sticky sticky_level, Component* comp) {
	std::vector<GameObject*> to_check{ this };
	while (!to_check.empty()) {
		GameObject* cur = to_check.back();
		to_check.pop_back();
		if (cur->comp_) {
			continue;
		}
		cur->comp_ = comp;
		comp->blocks_.push_back(cur);
		cur->collect_sticky_links(map, sticky_level, to_check);
		cur->collect_special_links(to_check);
	}
}

Sticky GameObject::sticky() {
	return Sticky::None;
}

bool GameObject::is_snake() {
	return false;
}

bool GameObject::has_sticky_neighbor(RoomMap* map) {
	return false;
}

void GameObject::collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&) {}

void GameObject::collect_special_links(std::vector<GameObject*>& to_check) {
	if (modifier_) {
		modifier_->collect_special_links(to_check);
	}
}
int GameObject::color() {
	return NO_COLOR;
}

FPoint3 GameObject::real_pos() {
	return pos_ + dpos_;
}

Block::Block(Point3 pos, bool pushable, bool gravitable) :
	GameObject(pos, pushable, gravitable) {}

Block::~Block() {}

void Block::draw_force_indicators(ModelInstancer& model, FPoint3 p, double radius) {
	if (!pushable_) {
		model.push_instance(glm::vec3(p.x, p.y, p.z - 0.2f), glm::vec3(radius, radius, 0.1f), BlockTexture::Blank, BLACK);
	}
	if (!gravitable_) {
		model.push_instance(glm::vec3(p.x, p.y, p.z + 0.2f), glm::vec3(radius, radius, 0.1f), BlockTexture::Blank, WHITE);
	}
}


ColoredBlock::ColoredBlock(Point3 pos, int color, bool pushable, bool gravitable) :
	Block(pos, pushable, gravitable), color_{ color } {}

ColoredBlock::~ColoredBlock() {}

int ColoredBlock::color() {
	return color_;
}


DestructionDelta::DestructionDelta(GameObject* obj) :
	obj_{ obj } {}

DestructionDelta::~DestructionDelta() {}

void DestructionDelta::revert() {
	obj_->undestroy();
}


AbstractShiftDelta::AbstractShiftDelta(GameObject* obj, Point3 dpos) :
	obj_{ obj }, dpos_{ dpos } {}

AbstractShiftDelta::~AbstractShiftDelta() {}

void AbstractShiftDelta::revert() {
	obj_->pos_ -= dpos_;
}


AbstractPutDelta::AbstractPutDelta(GameObject* obj, Point3 pos) :
	obj_{ obj }, pos_{ pos } {}

AbstractPutDelta::~AbstractPutDelta() {}

void AbstractPutDelta::revert() {
	obj_->pos_ = pos_;
}