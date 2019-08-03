#include "stdafx.h"
#include "switchable.h"

#include "delta.h"
#include "moveprocessor.h"
#include "signaler.h"

Switchable::Switchable(GameObject* parent, bool persistent, bool def, bool active, bool waiting) : ObjectModifier(parent),
persistent_{ persistent },
default_{ def },
active_{ active },
waiting_{ waiting },
signalers_{} {}

Switchable::~Switchable() {}

void Switchable::push_signaler(Signaler* signaler, int index) {
	signalers_.push_back(std::make_pair(signaler, index));
}

void Switchable::remove_signaler(Signaler* signaler) {
	signalers_.erase(std::remove_if(signalers_.begin(), signalers_.end(),
		[signaler](auto pair){return (pair.first == signaler);}),
		signalers_.end());
}

void Switchable::connect_to_signalers() {
	for (auto& p : signalers_) {
		p.first->push_switchable(this, p.second, false);
	}
}

bool Switchable::state() {
	return default_ ^ active_;
}

void Switchable::receive_signal(bool signal, RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if ((persistent_ && active_) || ((active_ ^ waiting_) == signal)) {
		return;
	}
	delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_));
	waiting_ = !can_set_state(default_ ^ signal, room_map);
	if (active_ != (waiting_ ^ signal)) {
		active_ = !active_;
		apply_state_change(room_map, delta_frame, mp);
	}
}

void Switchable::apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*) {}

void Switchable::check_waiting(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (waiting_ && can_set_state(!(default_ ^ active_), room_map)) {
		delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_));
		waiting_ = false;
		active_ = !active_;
		apply_state_change(room_map, delta_frame, mp);
	}
}

void Switchable::cleanup_on_destruction(RoomMap* room_map) {
	for (auto& p : signalers_) {
		p.first->remove_switchable(this, p.second);
	}
}

void Switchable::setup_on_undestruction(RoomMap* room_map) {
	connect_to_signalers();
}
