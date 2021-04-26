#include "stdafx.h"
#include "delta.h"

#include "mapfile.h"
#include "gameobject.h"
#include "playingstate.h"
#include "room.h"
#include "roommap.h"
#include "gatebody.h"
#include "gate.h"
#include "player.h"
#include "car.h"
#include "objectmodifier.h"
#include "gameobjectarray.h"

FrozenObject::FrozenObject(GameObject* obj) : obj_{ obj }, pos_{ obj->pos_ } {
	init_from_obj();
}

FrozenObject::FrozenObject(ObjectModifier* mod) : obj_{ mod->parent_ }, pos_{ mod->pos() } {
	init_from_obj();
}

FrozenObject::FrozenObject(Point3 pos, ObjRefCode ref) : obj_{ nullptr }, pos_{ pos }, ref_{ ref } {}

FrozenObject::~FrozenObject() {}

void FrozenObject::init_from_obj() {
	if (!obj_) {
		ref_ = ObjRefCode::Null;
		return;
	}
	if (obj_->tangible_) {
		ref_ = ObjRefCode::Tangible;
		return;
	}
	if (auto* gate_body = dynamic_cast<GateBody*>(obj_)) {
		if (auto* gate = gate_body->gate_) {
			if (gate->body_ == gate_body) {
				ref_ = ObjRefCode::HeldGateBody;
				return;
			}
		}
	}
	if (auto* player = dynamic_cast<Player*>(obj_)) {
		if (auto* car = player->car_) {
			if (car->player_ == player) {
				ref_ = ObjRefCode::HeldPlayer;
				return;
			}
		}
	}
	ref_ = ObjRefCode::Dead;
}

void FrozenObject::serialize(MapFileO& file, GameObjectArray* arr) {
	file << ref_;
	if (ref_ == ObjRefCode::Null) {
		return;
	}
	if (ref_ != ObjRefCode::Dead) {
		file << pos_;
	} else  {
		file.write_uint32(arr->dead_obj_map_[obj_]);
	}
}

GameObject* FrozenObject::resolve(RoomMap* room_map) {
	if (!obj_) {
		obj_ = room_map->deref_object(ref_, pos_);
	}
	return obj_;
}


Delta::~Delta() {}


DeltaFrame::DeltaFrame() {}

DeltaFrame::~DeltaFrame() {}

void DeltaFrame::revert(PlayingState* state) {
	for (auto it = deltas_.rbegin(); it != deltas_.rend(); ++it) {
		(**it).revert(state->room_->map());
	}
}

void DeltaFrame::push(std::unique_ptr<Delta> delta) {
	deltas_.push_back(std::move(delta));
	changed_ = true;
}

bool DeltaFrame::trivial() {
	return deltas_.empty();
}

void DeltaFrame::reset_changed() {
	changed_ = false;
}

bool DeltaFrame::changed() {
	return changed_;
}

void DeltaFrame::serialize(MapFileO& file, GameObjectArray* arr) {
	file.write_uint32((unsigned int)deltas_.size());
	for (auto& delta : deltas_) {
		delta->serialize(file, arr);
	}
}


UndoStack::UndoStack(PlayingState* state, unsigned int max_depth) :
	state_{ state }, frames_ {}, max_depth_{ max_depth }, size_{ 0 } {}

UndoStack::~UndoStack() {}

void UndoStack::push(std::unique_ptr<DeltaFrame> delta_frame) {
	if (!delta_frame->trivial()) {
		if (size_ == max_depth_) {
			frames_.pop_front();
		} else {
			++size_;
		}
		frames_.push_back(std::move(delta_frame));
	}
}

bool UndoStack::non_empty() {
	return size_ > 0;
}

void UndoStack::pop() {
	frames_.back()->revert(state_);
	frames_.pop_back();
	--size_;
}

void UndoStack::reset() {
	frames_.clear();
	size_ = 0;
}

void UndoStack::serialize(MapFileO& file, GameObjectArray* arr) {
	file.write_uint32((unsigned int)frames_.size());
	for (auto& frame : frames_) {
		frame->serialize(file, arr);
	}
}