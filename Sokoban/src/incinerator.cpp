#include "stdafx.h"
#include "incinerator.h"

#include "gameobject.h"
#include "mapfile.h"
#include "roommap.h"
#include "moveprocessor.h"
#include "animationmanager.h"
#include "color_constants.h"
#include "texture_constants.h"
#include "graphicsmanager.h"

Incinerator::Incinerator(GameObject* parent, int count, bool persistent, bool def, bool active) :
	Switchable(parent, count, persistent, def, active, false) {}


Incinerator::~Incinerator() {}

void Incinerator::make_str(std::string& str) {
	str += "Incinerator";
	Switchable::make_str(str);
}

ModCode Incinerator::mod_code() {
	return ModCode::Incinerator;
}

void Incinerator::serialize(MapFileO& file) {
	file << count_ << persistent_ << default_ << active_;
}

void Incinerator::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	unsigned char b[4];
	file.read(b, 4);
	parent->set_modifier(std::make_unique<Incinerator>(parent, b[0], b[1], b[2], b[3]));
}

bool Incinerator::can_set_state(bool state, RoomMap* map) {
	return true;
}

void Incinerator::map_callback(RoomMap*, DeltaFrame*, MoveProcessor* mp) {
	if (parent_->tangible_) {
		mp->alerted_incinerators_.push_back(this);
	}
}

void Incinerator::apply_state_change(RoomMap*, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (parent_->tangible_) {
		mp->alerted_incinerators_.push_back(this);
		if (state()) {
			mp->anims_->receive_signal(AnimationSignal::IncineratorOn, parent_, delta_frame);
		} else {
			mp->anims_->receive_signal(AnimationSignal::IncineratorOff, parent_, delta_frame);
		}
	}
}

void Incinerator::setup_on_put(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	Switchable::setup_on_put(map, delta_frame, real);
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
}

void Incinerator::cleanup_on_take(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	Switchable::cleanup_on_take(map, delta_frame, real);
	map->remove_listener(this, pos_above());
}

void Incinerator::draw(GraphicsManager* gfx, FPoint3 p) {
	int color = state() ? ORANGE : GREY;
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.9f, 0.9f, 0.1f), BlockTexture::Incinerator, color);
}

std::unique_ptr<ObjectModifier> Incinerator::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<Incinerator>(*this);
	dup->parent_ = parent;
	return std::move(dup);
}
