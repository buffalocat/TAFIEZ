#include "stdafx.h"
#include "gatebody.h"


#include "point.h"
#include "gate.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "moveprocessor.h"
#include "texture_constants.h"

#include "delta.h"

GateBody::GateBody(Gate* gate, Point3 pos) :
	PushBlock(pos, gate->color_, gate->pushable(), gate->gravitable(), Sticky::None),
	snake_{ gate->parent_->obj_code() == ObjCode::SnakeBlock },
	persistent_{ gate->persistent_ }, corrupt_{ false } {
	set_gate(gate);
}

GateBody::GateBody(Point3 pos, int color, bool pushable, bool gravitable,
	bool snake, bool persistent, bool corrupt) :
	PushBlock(pos, color, pushable, gravitable, Sticky::None),
	gate_{}, snake_{ snake }, persistent_{ persistent }, corrupt_{ corrupt } {}

GateBody::~GateBody() {}

std::string GateBody::name() {
	return "GateBody";
}

ObjCode GateBody::obj_code() {
	return ObjCode::GateBody;
}

void GateBody::serialize(MapFileO& file) {
	file << color_ << pushable_ << gravitable_ << snake_ << persistent_ << corrupt_;
}

std::unique_ptr<GameObject> GateBody::deserialize(MapFileI& file) {
	Point3 pos{ file.read_point3() };
	unsigned char b[6];
	file.read(b, 6);
	return std::make_unique<GateBody>(pos, b[0], b[1], b[2], b[3], b[4], b[5]);
}

bool GateBody::relation_check() {
	return gate_;
}

void GateBody::relation_serialize(MapFileO& file) {
	file << MapCode::GateBaseLocation;
	file << pos_;
	file << gate_->pos();
}

Point3 GateBody::gate_pos() {
	return gate_->pos();
}

void GateBody::set_gate(Gate* gate) {
	gate_ = gate;
	if (gate) {
		gate->body_ = this;
	}
}

void GateBody::destroy(MoveProcessor* mp, CauseOfDeath death) {
	if (gate_) {
		gate_->body_ = nullptr;
		mp->delta_frame_->push(std::make_unique<DestructionDelta>(this));
	}
}

void GateBody::undestroy() {
	if (gate_) {
		gate_->body_ = this;
	}
}

void GateBody::collect_special_links(std::vector<GameObject*>& to_check) {
	if (gate_) {
		to_check.push_back(gate_->parent_);
	}
}

const double GATE_ANIMATION_HEIGHTS[MAX_GATE_ANIMATION_FRAMES] = { 0.1, 0.3, 0.7, 0.9 };

void GateBody::draw(GraphicsManager* gfx) {
	FPoint3 p{ real_pos() };
	double height = 1.0f;
	if (gate_) {
		switch (gate_->animation_state_) {
		case GateAnimationState::Lower:
			height = GATE_ANIMATION_HEIGHTS[gate_->animation_time_];
			break;
		case GateAnimationState::Raise:
			height = 1 - GATE_ANIMATION_HEIGHTS[gate_->animation_time_];
			break;
		default:
			height = 1.0f;
			break;
		}
	}
	BlockTexture tex = corrupt_ ? BlockTexture::GateBodyCorrupt :
		(persistent_ ? BlockTexture::GateBodyPersistent : BlockTexture::GateBody);
	ModelInstancer& model = snake_ ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z - (1.0f - height) / 2),
		glm::vec3(0.6f, 0.6f, height), tex, color_);
	// Only draw belts if there's no parent Gate (this is a strange choice)
	if (!gate_) {
		draw_force_indicators(model, p, 0.75f);
	}
}
