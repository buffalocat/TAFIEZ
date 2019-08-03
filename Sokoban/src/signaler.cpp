#include "stdafx.h"
#include "signaler.h"

#include "switch.h"
#include "switchable.h"
#include "delta.h"
#include "mapfile.h"

Signaler::Signaler(const std::string& label, int count) :
	switches_{}, label_{ label }, prev_count_{ count }, count_{ count } {}

Signaler::~Signaler() {}

void Signaler::push_switch(Switch* obj, bool mutual) {
	if (obj) {
		switches_.push_back(obj);
		if (mutual) {
			obj->push_signaler(this);
		}
	}
}

void Signaler::remove_switch(Switch* obj) {
	switches_.erase(std::remove(switches_.begin(), switches_.end(), obj), switches_.end());
}

void Signaler::receive_signal(bool signal) {
	if (signal) {
		++count_;
	} else {
		--count_;
	}
}

void Signaler::update_count(DeltaFrame* delta_frame) {
	if (count_ != prev_count_) {
		delta_frame->push(std::make_unique<SignalerCountDelta>(this, prev_count_));
		prev_count_ = count_;
	}
}

void Signaler::reset_count(int count) {
	prev_count_ = count;
	count_ = count;
}

ThresholdSignaler::ThresholdSignaler(std::string label, int count, int threshold) :
	Signaler(label, count), switchables_{}, threshold_{ threshold } {}

ThresholdSignaler::~ThresholdSignaler() {}

void ThresholdSignaler::serialize(MapFileO& file) {
	file << MapCode::ThresholdSignaler;
	file << label_;
	file << count_ << threshold_;
	file << (unsigned int)switches_.size();
	file << (unsigned int)switchables_.size();
	for (auto& obj : switches_) {
		file << obj->pos();
	}
	for (auto& obj : switchables_) {
		file << obj->pos();
	}
}

void ThresholdSignaler::push_switchable(Switchable* obj, bool mutual, int) {
	switchables_.push_back(obj);
	if (mutual) {
		obj->push_signaler(this, 0);
	}
}

void ThresholdSignaler::remove_switchable(Switchable* obj, int) {
	switchables_.erase(std::remove(switchables_.begin(), switchables_.end(), obj), switchables_.end());
}

void ThresholdSignaler::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	bool state = count_ >= threshold_;
	if (state != (prev_count_ >= threshold_)) {
		for (Switchable* obj : switchables_) {
			obj->receive_signal(state, room_map, delta_frame, mp);
		}
	}
	update_count(delta_frame);
}


ParitySignaler::ParitySignaler(std::string label, int count, int parity_level) :
	Signaler(label, count), switchables_{}, parity_level_{ parity_level } {
	for (int i = 0; i < parity_level; ++i) {
		switchables_.push_back({});
	}
}

ParitySignaler::~ParitySignaler() {}

void ParitySignaler::serialize(MapFileO& file) {
	file << MapCode::ParitySignaler;
	file << label_;
	file << count_;
	file << (unsigned int)switches_.size();
	file << (unsigned int)switchables_.size();
	for (auto& obj : switches_) {
		file << obj->pos();
	}
	for (auto& group : switchables_) {
		file << (unsigned int)group.size();
		for (auto& obj : group) {
			file << obj->pos();
		}
	}
}

void ParitySignaler::push_switchable(Switchable* obj, bool mutual, int index) {
	switchables_[index].push_back(obj);
	if (mutual) {
		obj->push_signaler(this, 0);
	}
}

void ParitySignaler::remove_switchable(Switchable* obj, int index) {
	auto& cur = switchables_[index];
	cur.erase(std::remove(cur.begin(), cur.end(), obj), cur.end());
}

void ParitySignaler::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	int prev_state = prev_count_ % parity_level_;
	int state = count_ % parity_level_;
	if (state != prev_state) {
		for (Switchable* obj : switchables_[prev_state]) {
			obj->receive_signal(false, room_map, delta_frame, mp);
		}
		for (Switchable* obj : switchables_[state]) {
			obj->receive_signal(true, room_map, delta_frame, mp);
		}
	}
	update_count(delta_frame);
}