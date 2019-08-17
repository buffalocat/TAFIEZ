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
	PushBlock(pos, gate->color_, gate->pushable(), gate->gravitable(), Sticky::None) {
	set_gate(gate);
}

GateBody::GateBody(Point3 pos, int color, bool pushable, bool gravitable) :
	PushBlock(pos, color, pushable, gravitable, Sticky::None) {}

GateBody::~GateBody() {}

std::string GateBody::name() {
	return "GateBody";
}

ObjCode GateBody::obj_code() {
	return ObjCode::GateBody;
}

void GateBody::serialize(MapFileO& file) {
	file << color_ << pushable_ << gravitable_;
}

std::unique_ptr<GameObject> GateBody::deserialize(MapFileI& file) {
	Point3 pos{ file.read_point3() };
	unsigned char b[3];
	file.read(b, 3);
	auto gate_body = std::make_unique<GateBody>(pos, b[0], b[1], b[2]);
	return std::move(gate_body);
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

void GateBody::collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>& to_check) {
	if (gate_) {
		to_check.push_back(gate_->parent_);
	}
}

void GateBody::draw(GraphicsManager* gfx) {
	FPoint3 p{ real_pos() };
	// TODO: make this depend on the state animation
	double height = 1.0f;
	BlockTexture tex = gate_->persistent_ ? BlockTexture::GateBodyPersistent : BlockTexture::GateBody;
	gfx->top_cube.push_instance(glm::vec3(p.x, p.y, p.z - (1.0f - height) / 2),
		glm::vec3(0.7f, 0.7f, height), tex, color_);
}
