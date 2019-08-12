#include "stdafx.h"
#include "switch.h"

#include "roommap.h"
#include "signaler.h"

Switch::Switch(GameObject* parent, bool persistent, bool active): ObjectModifier(parent),
persistent_ {persistent}, active_ {active}, signalers_ {} {}

Switch::~Switch() {}

void Switch::push_signaler(Signaler* signaler) {
    signalers_.push_back(signaler);
}

void Switch::remove_signaler(Signaler* signaler) {
	signalers_.erase(std::remove(signalers_.begin(), signalers_.end(), signaler), signalers_.end());
}

void Switch::connect_to_signalers() {
    for (Signaler* s : signalers_) {
        s->push_switch(this, false);
    }
}

void Switch::toggle(bool propagate) {
    active_ = !active_;
	if (propagate) {
		for (auto& signaler : signalers_) {
			signaler->receive_signal(active_);
		}
	}
}

void Switch::cleanup_on_take(RoomMap* map, bool real) {
	if (real) {
		for (Signaler* s : signalers_) {
			s->remove_switch(this);
		}
	}
}

void Switch::setup_on_put(RoomMap* map, bool real) {
	if (real) {
		connect_to_signalers();
	}
}
