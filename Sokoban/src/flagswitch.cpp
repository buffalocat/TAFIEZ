#include "stdafx.h"
#include "flagswitch.h"

#include "gameobject.h"
#include "player.h"
#include "car.h"
#include "mapfile.h"
#include "moveprocessor.h"
#include "animationmanager.h"
#include "graphicsmanager.h"
#include "texture_constants.h"
#include "roommap.h"


FlagSwitch::FlagSwitch(GameObject* parent, bool active, int orientation) :
	PressSwitch(parent, 0, false, active), orientation_{ orientation } {}

FlagSwitch::~FlagSwitch() {}

void FlagSwitch::make_str(std::string& str) {
	str += "FlagSwitch";
	Switch::make_str(str);
}

ModCode FlagSwitch::mod_code() {
	return ModCode::FlagSwitch;
}

void FlagSwitch::serialize(MapFileO& file) {
	file << active_ << orientation_;
}

void FlagSwitch::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	unsigned char b[2];
	file.read(b, 2);
	parent->set_modifier(std::make_unique<FlagSwitch>(parent, b[0], b[1]));
}

bool FlagSwitch::should_toggle(RoomMap* map) {
	bool player_on = false;
	if (auto* above = map->view(pos_above())) {
		if (dynamic_cast<Player*>(above)) {
			player_on = true;
		} else if (auto* car = dynamic_cast<Car*>(above->modifier())) {
			if (car->player_) {
				player_on = true;
			}
		}
	}
	return active_ ^ (map->view(pos_above()) != nullptr);
}

void FlagSwitch::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (parent_->tangible_) {
		if (check_send_signal(map, delta_frame)) {
			if (active_) {
				mp->anims_->receive_signal(AnimationSignal::FlagSwitchOn, parent_, nullptr);
			} else {
				mp->anims_->receive_signal(AnimationSignal::FlagSwitchOff, parent_, nullptr);
			}
		}
	}
}

void FlagSwitch::draw(GraphicsManager* gfx, FPoint3 p) {
	BlockTexture tex = static_cast<BlockTexture>((int)BlockTexture::FlagSwitch0 + orientation_);
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.9f, 0.9f, 0.1f), tex, glm::vec4(1.0f));
}

std::unique_ptr<ObjectModifier> FlagSwitch::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<FlagSwitch>(*this);
	dup->parent_ = parent;
	dup->connect_to_signalers();
	return std::move(dup);
}
