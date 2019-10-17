#include "stdafx.h"
#include "switchable.h"

#include "delta.h"
#include "moveprocessor.h"
#include "signaler.h"

Switchable::Switchable(GameObject* parent, int count, bool persistent, bool def, bool active, bool waiting) : ObjectModifier(parent),
count_ { count },
persistent_{ persistent },
default_{ def },
active_{ active },
waiting_{ waiting },
signalers_{} {}

Switchable::~Switchable() {}

void Switchable::make_str(std::string& str) {
	if (signalers_.size()) {
		std::string sig_list{ "(" };
		for (auto& p : signalers_) {
			if (p.first) {
				sig_list += p.first->label_ + "; ";
			}
		}
		sig_list += ")";
		str += sig_list;
	}
}

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

//TODO : move Delta creation into check_active_change (requires keeping better track of count, like with signalers)

void Switchable::receive_signal(bool signal, RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	delta_frame->push(std::make_unique<SwitchableDelta>(this, count_, active_, waiting_));
	if (signal) {
		++count_;
	} else {
		--count_;
	}
	mp->activated_switchables_.insert(this);
}

void Switchable::check_active_change(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (persistent_ && (active_ ^ waiting_)) {
		if (waiting_ && can_set_state(default_ ^ 1, map)) {
			active_ = true;
			waiting_ = false;
			apply_state_change(map, delta_frame, mp);
		}
		return;
	}
	bool cur_signal = count_ > 0;
	if (active_ == cur_signal) {
		waiting_ = false;
		return;
	}
	waiting_ = !can_set_state(default_ ^ cur_signal, map);
	if (active_ != (waiting_ ^ cur_signal)) {
		active_ = !active_;
		apply_state_change(map, delta_frame, mp);
	}
}

void Switchable::apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*) {}

void Switchable::check_waiting(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (waiting_) {
		delta_frame->push(std::make_unique<SwitchableDelta>(this, count_, active_, waiting_));
		mp->activated_switchables_.insert(this);
	}
}

void Switchable::cleanup_on_take(RoomMap* map, bool real) {
	if (real) {
		for (auto& p : signalers_) {
			p.first->remove_switchable(this, p.second);
		}
	}
}

void Switchable::setup_on_put(RoomMap* map, bool real) {
	if (real) {
		connect_to_signalers();
	}
}
