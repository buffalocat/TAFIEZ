#include "stdafx.h"
#include "switch.h"

#include "roommap.h"
#include "signaler.h"
#include "gameobject.h"

Switch::Switch(GameObject* parent, bool persistent, bool active): ObjectModifier(parent),
persistent_ {persistent}, active_ {active}, signalers_ {} {}

Switch::~Switch() {}

void Switch::make_str(std::string& str) {
	if (signalers_.size()) {
		std::string sig_list{ "(" };
		for (auto* sig : signalers_) {
			if (sig) {
				sig_list += sig->label_ + "; ";
			}
		}
		sig_list += ")";
		str += sig_list;
	}
}

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

void Switch::remove_from_signalers() {
	for (Signaler* s : signalers_) {
		s->remove_switch(this);
	}
}

void Switch::toggle() {
    active_ = !active_;
	send_signal(active_);
}

void Switch::send_signal(bool signal) {
	for (auto& signaler : signalers_) {
		signaler->receive_signal(signal);
	}
}

void Switch::cleanup_on_take(RoomMap* map, DeltaFrame*, bool real) {
	if (real) {
		remove_from_signalers();
		if (active_) {
			send_signal(false);
		}
	}
}

void Switch::setup_on_put(RoomMap* map, DeltaFrame*, bool real) {
	if (real) {
		connect_to_signalers();
		if (active_) {
			send_signal(true);
		}
	}
}


SwitchToggleDelta::SwitchToggleDelta(Switch* obj) : obj_{ obj } {}

SwitchToggleDelta::SwitchToggleDelta(FrozenObject obj) : obj_{ obj } {}

SwitchToggleDelta::~SwitchToggleDelta() {}

void SwitchToggleDelta::serialize(MapFileO& file) {
	obj_.serialize(file);
}

void SwitchToggleDelta::revert(RoomMap* room_map) {
	static_cast<Switch*>(obj_.resolve_mod(room_map))->toggle();
}

DeltaCode SwitchToggleDelta::code() {
	return DeltaCode::SwitchToggleDelta;
}

std::unique_ptr<Delta> SwitchToggleDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<SwitchToggleDelta>(file.read_frozen_obj());
}
