#include "stdafx.h"
#include "pressswitch.h"

#include "gameobject.h"
#include "mapfile.h"
#include "roommap.h"
#include "delta.h"
#include "graphicsmanager.h"
#include "texture_constants.h"


PressSwitch::PressSwitch(GameObject* parent, int color, bool persistent, bool active):
Switch(parent, persistent, active), color_ {color} {}

PressSwitch::~PressSwitch() {}

void PressSwitch::make_str(std::string& str) {
	str += "PressSwitch";
	Switch::make_str(str);
}

ModCode PressSwitch::mod_code() {
    return ModCode::PressSwitch;
}

void PressSwitch::serialize(MapFileO& file) {
    file << color_ << persistent_ << active_;
}

void PressSwitch::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
    unsigned char b[3];
    file.read(b, 3);
    parent->set_modifier(std::make_unique<PressSwitch>(parent, b[0], b[1], b[2]));
}

void PressSwitch::check_send_signal(RoomMap* map, DeltaFrame* delta_frame) {
    if (active_ && persistent_) {
        return;
    }
    if (should_toggle(map)) {
        delta_frame->push(std::make_unique<SwitchToggleDelta>(this));
        toggle();
    }
}

bool PressSwitch::should_toggle(RoomMap* map) {
    return active_ ^ (map->view(pos_above()) != nullptr);
}

void PressSwitch::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (parent_->tangible_) {
		check_send_signal(map, delta_frame);
	}
}

void PressSwitch::setup_on_put(RoomMap* map, bool real) {
	Switch::setup_on_put(map, real);
    map->add_listener(this, pos_above());
    map->activate_listener_of(this);
}

void PressSwitch::cleanup_on_take(RoomMap* map, bool real) {
	Switch::cleanup_on_take(map, real);
    map->remove_listener(this, pos_above());
}

void PressSwitch::draw(GraphicsManager* gfx, FPoint3 p) {
	BlockTexture tex;
	if (persistent_) {
		if (active_) {
			tex = BlockTexture::SwitchDown;
		}
		else {
			tex = BlockTexture::SwitchUp;
		}
	}
	else {
		tex = BlockTexture::Cross;
	}
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.8f, 0.8f, 0.1f), tex, color_);
}

std::unique_ptr<ObjectModifier> PressSwitch::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    auto dup = std::make_unique<PressSwitch>(*this);
    dup->parent_ = parent;
    dup->connect_to_signalers();
    return std::move(dup);
}
