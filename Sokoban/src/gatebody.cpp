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
	file << gate_pos_;
}

Point3 GateBody::gate_pos() {
	return gate_pos_;
}

void GateBody::set_gate(Gate* gate) {
	gate_ = gate;
	if (gate) {
		gate->body_ = this;
		gate_pos_ = gate->pos();
	}
}

Point3 GateBody::update_gate_pos(DeltaFrame* delta_frame) {
	Point3 dpos = gate_->pos() - gate_pos_;
	if (!(dpos == Point3{})) {
		delta_frame->push(std::make_unique<GatePosDelta>(this, dpos));
		gate_pos_ = gate_->pos();
	}
	return dpos;
}

void GateBody::destroy(DeltaFrame* delta_frame, CauseOfDeath) {
	if (gate_) {
		gate_->body_ = nullptr;
		delta_frame->push(std::make_unique<DestructionDelta>(this));
	}
}

void GateBody::undestroy() {
	if (gate_) {
		gate_->body_ = this;
	}
}

void GateBody::collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>& to_check) {
	if (gate_) {
		to_check.push_back(gate_->parent_);
	}
}

void GateBody::draw(GraphicsManager* gfx) {
	FPoint3 p{ real_pos() };
	// TODO: make this depend on the state animation
	double height = 1.0f;
	BlockTexture tex = corrupt_ ? BlockTexture::GateBodyCorrupt :
		(persistent_ ? BlockTexture::GateBodyPersistent : BlockTexture::GateBody);
	ModelInstancer& model = snake_ ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z - (1.0f - height) / 2),
		glm::vec3(0.7f, 0.7f, height), tex, color_);
}