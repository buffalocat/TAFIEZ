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

FrozenObject::FrozenObject() :
	obj_{ nullptr }, ref_{ ObjRefCode::Null }, inacc_id_{ 0 } {}

FrozenObject::FrozenObject(GameObject* obj) :
	obj_{ obj } {
	init_from_obj();
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

FrozenObject::FrozenObject(Point3 pos, ObjRefCode ref, unsigned int inacc_id) :
	obj_{ nullptr }, pos_{ pos }, ref_{ ref }, inacc_id_{ inacc_id } {}

FrozenObject::~FrozenObject() {}

void FrozenObject::init_from_obj() {
	if (!obj_) {
		ref_ = ObjRefCode::Null;
		return;
	}
	inacc_id_ = obj_->inacc_id_;
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
	std::cout << "Made Inaccessible FrozenObject" << std::endl;
	pos_ = obj_->pos_;
	ref_ = ObjRefCode::Inaccessible;
}

void FrozenObject::serialize(MapFileO& file) {
	file << ref_;
	file.write_uint32(inacc_id_);
	switch (ref_) {
	case ObjRefCode::Null:
	case ObjRefCode::Inaccessible:
		break;
	default:
		file << pos_;
		break;
	}
}

GameObject* FrozenObject::resolve(RoomMap* room_map) {
	if (!obj_) {
		obj_ = room_map->deref_object(this);
	}
	return obj_;
}

ObjectModifier* FrozenObject::resolve_mod(RoomMap* room_map) {
	if (!obj_) {
		obj_ = room_map->deref_object(this);
		if (!obj_) {
			return nullptr;
		}
	}
	return obj_->modifier();
}

void FrozenObject::print() {
	if (ref_ == ObjRefCode::Tangible) {
		Point3 p = pos_;
		std::cout << "tangible at " << p.x << " " << p.y << " " << p.z;
	} else {
		std::cout << "key " << inacc_id_;
	}
}

Delta::~Delta() {}


DeltaFrame::DeltaFrame() {}

DeltaFrame::~DeltaFrame() {}

void DeltaFrame::revert(PlayingState* state) {
	for (auto it = deltas_.rbegin(); it != deltas_.rend(); ++it) {
		(**it).revert(state->room_->map());
	}
	deltas_.clear();
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
    push(std::move(CLASS::deserialize(file)));\
    break;

void DeltaFrame::deserialize(MapFileIwithObjs& file) {
	unsigned int n_deltas = file.read_uint32();
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
			CASE_DELTACODE(MotionDelta);
			CASE_DELTACODE(BatchMotionDelta);
			CASE_DELTACODE(ClearFlagCollectionDelta);
			CASE_DELTACODE(AddPlayerDelta);
			CASE_DELTACODE(RemovePlayerDelta);
			CASE_DELTACODE(CyclePlayerDelta);
			CASE_DELTACODE(GlobalFlagDelta);
			CASE_DELTACODE(FlagCountDelta);
			CASE_DELTACODE(SignalerCountDelta);
			CASE_DELTACODE(AddLinkDelta);
			CASE_DELTACODE(RemoveLinkDelta);
			CASE_DELTACODE(GateUnlinkDelta);
			CASE_DELTACODE(SwitchToggleDelta);
			CASE_DELTACODE(SwitchableDelta);
		}
	}
}

#undef CASE_DELTACODE


void DeltaFrame::serialize(MapFileO& file) {
	file.write_uint32((unsigned int)deltas_.size());
	for (auto& delta : deltas_) {
		file << delta->code();
		delta->serialize(file);
	}
}


UndoStack::UndoStack(PlayingState* state, unsigned int max_depth) :
	state_{ state }, max_depth_{ max_depth } {}

UndoStack::~UndoStack() {}

void UndoStack::push(std::unique_ptr<DeltaFrame> delta_frame) {
	if (delta_frame->trivial()) {
		return;
	}
	frames_.push_back(std::move(delta_frame));
	if (size_ == max_depth_) {
		frames_.pop_front();
		if (!cache_map_.empty()) {
			++num_new_frames_;
			if (++skip_frames_ == cache_map_.front().second) {
				std::cout << "POP" << std::endl;
				cache_map_.pop_front();
				skip_frames_ = 0;
			}
		}
	} else {
		++num_new_frames_;
		++size_;
	}
}

bool UndoStack::non_empty() {
	return size_ > 0;
}

void UndoStack::pop() {
	if (num_new_frames_ > 0) {
		--num_new_frames_;
	} else if (--cache_map_.back().second == 0) {
		cache_map_.pop_back();
	}
	frames_.back()->revert(state_);
	frames_.pop_back();
	--size_;
}

void UndoStack::reset() {
	frames_.clear();
	size_ = 0;
	num_new_frames_ = 0;
	skip_frames_ = 0;
	cache_map_.clear();
}


void UndoStack::deserialize(std::filesystem::path base_path, unsigned int subsave_index, GameObjectArray* arr) {
	MapFileI meta_file{ base_path / std::to_string(subsave_index) / "delta_meta.sav" };
	unsigned int n_cache_map = meta_file.read_uint32();
	for (unsigned int i_group = 0; i_group < n_cache_map; ++i_group) {
		unsigned int index = meta_file.read_uint32();
		unsigned int count = meta_file.read_uint32();
		cache_map_.push_back(std::make_pair(index, count));
		MapFileIwithObjs file{ base_path / std::to_string(index) / "deltas.sav", arr };
		for (unsigned int i_frame = 0; i_frame < count; ++i_frame) {
			auto frame = std::make_unique<DeltaFrame>();
			frame->deserialize(file);
			push(std::move(frame));
		}
	}
	num_new_frames_ = 0;
}

void UndoStack::serialize(std::filesystem::path subsave_path, unsigned int subsave_index, GameObjectArray* arr) {
	MapFileO meta_file{ subsave_path / "delta_meta.sav" };
	if (num_new_frames_ > 0) {
		cache_map_.push_back(std::make_pair(subsave_index, num_new_frames_));
	}
	meta_file.write_uint32((unsigned int)cache_map_.size());
	unsigned int start_frame = 0;
	for (auto& p : cache_map_) {
		meta_file.write_uint32(p.first);
		meta_file.write_uint32(p.second);
		start_frame += p.second;
	}
	start_frame = start_frame - skip_frames_ - num_new_frames_;
	MapFileO file{ subsave_path / "deltas.sav" };
	for (auto it = frames_.begin() + start_frame; it != frames_.end(); ++it) {
		(*it)->serialize(file);
	}
	num_new_frames_ = 0;
}

std::vector<unsigned int> UndoStack::dependent_subsaves() {
	std::vector<unsigned int> dep{};
	for (auto& p : cache_map_) {
		dep.push_back(p.first);
	}
	return dep;
}
