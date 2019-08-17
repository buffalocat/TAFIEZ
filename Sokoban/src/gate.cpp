#include "stdafx.h"
#include "gate.h"

#include "point.h"

#include "gatebody.h"
#include "mapfile.h"
#include "roommap.h"

#include "moveprocessor.h"
#include "graphicsmanager.h"
#include "texture_constants.h"

Gate::Gate(GameObject* parent, GateBody* body, int color, int count, bool persistent, bool def, bool active, bool waiting) :
	Switchable(parent, count, persistent, def, active, waiting), color_{ color }, body_{ body } {}

Gate::~Gate() {}

std::string Gate::name() {
	return "Gate";
}

ModCode Gate::mod_code() {
	return ModCode::Gate;
}

void Gate::serialize(MapFileO& file) {
	bool holding_body = body_ && !state();
	file << color_ << count_ << persistent_ << default_ << active_ << waiting_ << holding_body;
	if (holding_body) {
		file << Point3_S16{ body_->pos_ - pos() };
	}
}

void Gate::deserialize(MapFileI& file, RoomMap* map, GameObject* parent) {
	unsigned char b[7];
	file.read(b, 7);
	auto gate = std::make_unique<Gate>(parent, nullptr, b[0], b[1], b[2], b[3], b[4], b[5]);
	// Is the body alive and retracted?
	if (b[6]) {
		Point3_S16 body_pos{};
		file >> body_pos;
		map->push_to_object_array(std::make_unique<GateBody>(gate.get(), Point3{ body_pos } +parent->pos_), nullptr);
	}
	parent->set_modifier(std::move(gate));
}

void Gate::shift_internal_pos(Point3 d) {
	if ((body_ != nullptr) && !state()) {
		body_->shift_internal_pos(d);
	}
}

void Gate::collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& to_check) {
	if (body_ && state()) {
		to_check.push_back(body_);
	}
}

bool Gate::can_set_state(bool state, RoomMap* map) {
	return body_ && (!state || (map->view(body_->pos_) == nullptr));
}

void Gate::apply_state_change(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (body_) {
		// Add animation
		if (state()) {
			map->put_in_map(body_, true, true, delta_frame);
		} else {
			map->take_from_map(body_, true, true, delta_frame);
			GameObject* above = map->view(body_->pos_ + Point3{ 0,0,1 });
			if (above && above->gravitable_) {
				mp->add_to_fall_check(above);
			}
		}
	}
}

void Gate::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (body_) {
		Point3 dpos = body_->update_gate_pos(delta_frame);
		if (!state()) {
			body_->abstract_shift(dpos, delta_frame);
		}
		// Is there a need to do this if the body is destroyed?
		check_waiting(map, delta_frame, mp);
	}
}

void Gate::setup_on_put(RoomMap* map, bool real) {
	Switchable::setup_on_put(map, real);
	if (body_) {
		map->add_listener(this, body_->pos_);
		map->activate_listener_of(this);
	}
}

void Gate::cleanup_on_take(RoomMap* map, bool real) {
	Switchable::cleanup_on_take(map, real);
	if (body_) {
		map->remove_listener(this, body_->pos_);
	}
}

// Consider making the Gate(base) visually different when the gate is active (only visible if the GateBody has been separated)
void Gate::draw(GraphicsManager* gfx, FPoint3 p) {
	BlockTexture tex = persistent_ ? BlockTexture::GateBasePersistent : BlockTexture::GateBase;
	gfx->top_cube.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.8f, 0.8f, 0.1f), tex, color_);
}

std::unique_ptr<ObjectModifier> Gate::duplicate(GameObject* parent, RoomMap* map, DeltaFrame* delta_frame) {
	auto dup = std::make_unique<Gate>(*this);
	dup->parent_ = parent;
	if (body_) {
		auto body_dup = std::make_unique<GateBody>(dup.get(), body_->pos_);
		dup->body_ = body_dup.get();
		// A GateBody can't be duplicated unless it's intangible
		map->push_to_object_array(std::move(body_dup), delta_frame);
	}
	dup->connect_to_signalers();
	return std::move(dup);
}
