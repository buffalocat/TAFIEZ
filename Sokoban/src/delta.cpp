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
#include "animationmanager.h"
#include "autosavepanel.h"
#include "flaggate.h"
#include "floorsign.h"
#include "clearflag.h"
#include "moveprocessor.h"
#include "savefile.h"
#include "switch.h"
#include "snakeblock.h"
#include "signaler.h"

FrozenObject::FrozenObject() {}

FrozenObject::FrozenObject(GameObject* obj) : obj_{ obj } {
	init_from_obj();
}

FrozenObject FrozenObject::create_dead_obj(GameObject* obj) {
	auto f_obj = FrozenObject();
	f_obj.obj_ = obj;
	f_obj.ref_ = ObjRefCode::Dead;
	return f_obj;
}

FrozenObject::FrozenObject(ObjectModifier* mod) {
	if (mod) {
		obj_ = mod->parent_;
		pos_ = obj_->pos_;
	} else {
		obj_ = nullptr;
		pos_ = { 0,0,0 };
	}
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
		pos_ = obj_->pos_;
		ref_ = ObjRefCode::Tangible;
		return;
	}
	if (auto* gate_body = dynamic_cast<GateBody*>(obj_)) {
		if (auto* gate = gate_body->gate_) {
			if (gate->body_ == gate_body) {
				pos_ = gate->pos();
				ref_ = ObjRefCode::HeldGateBody;
				return;
			}
		}
	}
	if (auto* player = dynamic_cast<Player*>(obj_)) {
		if (auto* car = player->car_) {
			if (car->player_ == player) {
				pos_ = car->pos();
				ref_ = ObjRefCode::HeldPlayer;
				return;
			}
		}
	}
	pos_ = obj_->pos_;
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
	if (ref_ == ObjRefCode::Null) {
		return nullptr;
	}
	if (!obj_) {
		obj_ = room_map->deref_object(ref_, pos_);
	}
	return obj_;
}

ObjectModifier* FrozenObject::resolve_mod(RoomMap* room_map) {
	if (ref_ == ObjRefCode::Null) {
		return nullptr;
	}
	if (!obj_) {
		obj_ = room_map->deref_object(ref_, pos_);
	}
	return obj_->modifier();
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


#define CASE_DELTACODE(CLASS)\
case DeltaCode::CLASS:\
    delta = CLASS::deserialize(file);\
    break;

void DeltaFrame::deserialize(MapFileIwithObjs& file) {
	unsigned int n_deltas = file.read_uint32();
	std::unique_ptr<Delta> delta{};
	for (unsigned int i = 0; i < n_deltas; ++i) {
		switch (static_cast<DeltaCode>(file.read_byte())) {
			CASE_DELTACODE(AnimationSignalDelta);
			CASE_DELTACODE(AutosavePanelDelta);
			CASE_DELTACODE(ClearFlagToggleDelta);
			CASE_DELTACODE(FlagGateOpenDelta);
			CASE_DELTACODE(SignToggleDelta);
			CASE_DELTACODE(LearnFlagDelta);
			CASE_DELTACODE(DestructionDelta);
			CASE_DELTACODE(AbstractShiftDelta);
			CASE_DELTACODE(AbstractPutDelta);
			CASE_DELTACODE(RoomChangeDelta);
			CASE_DELTACODE(ToggleGravitableDelta);
			CASE_DELTACODE(ColorChangeDelta);
			CASE_DELTACODE(ModDestructionDelta);
			CASE_DELTACODE(PlayerStateDelta);
			CASE_DELTACODE(PutDelta);
			CASE_DELTACODE(TakeDelta);
			CASE_DELTACODE(WallDestructionDelta);
			CASE_DELTACODE(ObjArrayPushDelta);
			CASE_DELTACODE(ObjArrayDeletedPushDelta);
			CASE_DELTACODE(MotionDelta);
			CASE_DELTACODE(BatchMotionDelta);
			CASE_DELTACODE(ClearFlagCollectionDelta);
			CASE_DELTACODE(AddPlayerDelta);
			CASE_DELTACODE(RemovePlayerDelta);
			CASE_DELTACODE(CyclePlayerDelta);
			CASE_DELTACODE(GlobalFlagDelta);
			CASE_DELTACODE(FlagCountDelta);
			CASE_DELTACODE(AutosaveDelta);
			CASE_DELTACODE(SignalerCountDelta);
			CASE_DELTACODE(AddLinkDelta);
			CASE_DELTACODE(RemoveLinkDelta);
			CASE_DELTACODE(RemoveLinkOneWayDelta);
			CASE_DELTACODE(SwitchToggleDelta);
			CASE_DELTACODE(SwitchableDelta);
		}
		push(std::move(delta));
	}
}

#undef CASE_DELTACODE


void DeltaFrame::serialize(MapFileO& file, GameObjectArray* arr) {
	file.write_uint32((unsigned int)deltas_.size());
	for (auto& delta : deltas_) {
		file << delta->code();
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

void UndoStack::deserialize(MapFileIwithObjs& file) {
	unsigned int n_frames = file.read_uint32();
	for (unsigned int i = 0; i < n_frames; ++i) {
		auto frame = std::make_unique<DeltaFrame>();
		frame->deserialize(file);
		push(std::move(frame));
	}
}

void UndoStack::serialize(MapFileO& file, GameObjectArray* arr) {
	file.write_uint32((unsigned int)frames_.size());
	for (auto& frame : frames_) {
		frame->serialize(file, arr);
	}
}