#include "stdafx.h"
#include "delta.h"

#include "gameobject.h"
#include "graphicsmanager.h"
#include "room.h"
#include "roommap.h"
#include "savefile.h"

#include "car.h"
#include "snakeblock.h"
#include "player.h"
#include "switch.h"
#include "switchable.h"
#include "signaler.h"
#include "playingstate.h"
#include "gatebody.h"
#include "clearflag.h"
#include "worldresetkey.h"
#include "floorsign.h"


Delta::~Delta() {}

DeltaFrame::DeltaFrame() {}

DeltaFrame::~DeltaFrame() {}

void DeltaFrame::revert() {
	for (auto it = deltas_.rbegin(); it != deltas_.rend(); ++it) {
		(**it).revert();
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


UndoStack::UndoStack(unsigned int max_depth) : frames_{}, max_depth_{ max_depth }, size_{ 0 } {}

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
	frames_.back()->revert();
	frames_.pop_back();
	--size_;
}

void UndoStack::reset() {
	frames_.clear();
	size_ = 0;
}


ObjArrayPushDelta::ObjArrayPushDelta(GameObject* obj, RoomMap* map) : obj_{ obj }, map_{ map } {}

ObjArrayPushDelta::~ObjArrayPushDelta() {}

void ObjArrayPushDelta::revert() {
	map_->remove_from_object_array(obj_);
}


PutDelta::PutDelta(GameObject* obj, RoomMap* map) :
	obj_{ obj }, map_{ map } {}

PutDelta::~PutDelta() {}

void PutDelta::revert() {
	map_->take_from_map(obj_, true, false, nullptr);
}


TakeDelta::TakeDelta(GameObject* obj, RoomMap* map) :
	obj_{ obj }, map_{ map } {}

TakeDelta::~TakeDelta() {}

void TakeDelta::revert() {
	map_->put_in_map(obj_, true, false, nullptr);
}


MotionDelta::MotionDelta(GameObject* obj, Point3 dpos, RoomMap* map) :
	obj_{ obj }, dpos_{ dpos }, map_{ map } {}

MotionDelta::~MotionDelta() {}

void MotionDelta::revert() {
	map_->shift(obj_, -dpos_, false, nullptr);
}


BatchMotionDelta::BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos, RoomMap* map) :
	objs_{ objs }, dpos_{ dpos }, map_{ map } {}

BatchMotionDelta::~BatchMotionDelta() {}

void BatchMotionDelta::revert() {
	map_->batch_shift(objs_, -dpos_, false, nullptr);
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


AddLinkDelta::AddLinkDelta(SnakeBlock* a, SnakeBlock* b) : a_{ a }, b_{ b } {}

AddLinkDelta::~AddLinkDelta() {}

void AddLinkDelta::revert() {
	a_->remove_link_quiet(b_);
}


RemoveLinkDelta::RemoveLinkDelta(SnakeBlock* a, SnakeBlock* b) : a_{ a }, b_{ b } {}

RemoveLinkDelta::~RemoveLinkDelta() {}

void RemoveLinkDelta::revert() {
	a_->add_link_quiet(b_);
}


RoomChangeDelta::RoomChangeDelta(PlayingState* state, Room* room) :
	state_{ state }, room_{ room } {}

RoomChangeDelta::~RoomChangeDelta() {}

void RoomChangeDelta::revert() {
	state_->activate_room(room_);
}


SwitchableDelta::SwitchableDelta(Switchable* obj, int count, bool active, bool waiting) :
	obj_{ obj }, count_{ count }, active_{ active }, waiting_{ waiting } {}

SwitchableDelta::~SwitchableDelta() {}

void SwitchableDelta::revert() {
	obj_->count_ = count_;
	obj_->active_ = active_;
	obj_->waiting_ = waiting_;
}


SwitchToggleDelta::SwitchToggleDelta(Switch* obj) : obj_{ obj } {}

SwitchToggleDelta::~SwitchToggleDelta() {}

void SwitchToggleDelta::revert() {
	obj_->toggle();
}


SignalerCountDelta::SignalerCountDelta(Signaler* sig, int count) : sig_{ sig }, count_{ count } {}

SignalerCountDelta::~SignalerCountDelta() {}

void SignalerCountDelta::revert() {
	sig_->reset_prev_count(count_);
}


PlayerStateDelta::PlayerStateDelta(Player* player, Car* car, PlayerState state) :
	player_{ player }, car_{ car }, state_{ state } {}

PlayerStateDelta::~PlayerStateDelta() {}

void PlayerStateDelta::revert() {
	player_->state_ = state_;
	player_->car_ = car_;
}


ColorChangeDelta::ColorChangeDelta(Car* car, bool undo) : car_{ car }, undo_{ undo } {}

ColorChangeDelta::~ColorChangeDelta() {}

void ColorChangeDelta::revert() {
	car_->cycle_color(undo_);
}


GatePosDelta::GatePosDelta(GateBody* gate_body, Point3 dpos) :
	gate_body_{ gate_body }, dpos_{ dpos } {}

GatePosDelta::~GatePosDelta() {}

void GatePosDelta::revert() {
	gate_body_->gate_pos_ -= dpos_;
}


ClearFlagToggleDelta::ClearFlagToggleDelta(ClearFlag* flag, RoomMap* map) :
	flag_{ flag }, map_{ map } {}

ClearFlagToggleDelta::~ClearFlagToggleDelta() {}

void ClearFlagToggleDelta::revert() {
	flag_->active_ = !flag_->active_;
	map_->clear_flags_[flag_] = flag_->active_;
}


ClearFlagCollectionDelta::ClearFlagCollectionDelta(RoomMap* map, int req) :
	map_{ map }, req_{ req } {}

ClearFlagCollectionDelta::~ClearFlagCollectionDelta() {}

void ClearFlagCollectionDelta::revert() {
	map_->uncollect_flag(req_);
}


KeyCollectDelta::KeyCollectDelta(WorldResetKey* key) :
	Delta(), key_{ key } {}

KeyCollectDelta::~KeyCollectDelta() {}

void KeyCollectDelta::revert() {
	key_->collected_ = false;
}


GlobalFlagDelta::GlobalFlagDelta(PlayingGlobalData* global, unsigned int flag):
	Delta(), global_{ global }, flag_{ flag } {}

GlobalFlagDelta::~GlobalFlagDelta() {}

void GlobalFlagDelta::revert() {
	global_->remove_flag(flag_);
}


SignToggleDelta::SignToggleDelta(FloorSign* sign, TextRenderer* text) :
	Delta(), sign_{ sign }, text_{ text } {}

SignToggleDelta::~SignToggleDelta() {}

void SignToggleDelta::revert() {
	sign_->toggle_active(text_, nullptr);
}


MapInitDelta::MapInitDelta(RoomMap* map) : Delta(), map_{ map } {}

MapInitDelta::~MapInitDelta() {}

void MapInitDelta::revert() {
	map_->inited_ = false;
}