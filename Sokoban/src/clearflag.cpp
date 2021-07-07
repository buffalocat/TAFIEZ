#include "stdafx.h"
#include "clearflag.h"

#include "gameobject.h"
#include "mapfile.h"
#include "moveprocessor.h"
#include "graphicsmanager.h"
#include "animationmanager.h"
#include "texture_constants.h"
#include "roommap.h"
#include "player.h"
#include "delta.h"
#include "car.h"
#include "roommap.h"
#include "savefile.h"
#include "globalflagconstants.h"

ClearFlag::ClearFlag(GameObject* parent, bool real, bool active, bool collected) :
	ObjectModifier(parent),
	real_{ real }, active_{ active }, collected_{ collected } {}

ClearFlag::~ClearFlag() {}

void ClearFlag::make_str(std::string& str) {
	char buf[32];
	snprintf(buf, 32, "ClearFlag:%s", real_ ? "Real" : "Fake");
	str += buf;
}

ModCode ClearFlag::mod_code() {
	return ModCode::ClearFlag;
}

void ClearFlag::serialize(MapFileO& file) {
	file << real_;
	file << active_;
}

void ClearFlag::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	bool real, active;
	file >> real >> active;
	auto cf = std::make_unique<ClearFlag>(parent, real, active, false);
	parent->set_modifier(std::move(cf));
}

std::unique_ptr<ObjectModifier> ClearFlag::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<ClearFlag>(*this);
	dup->parent_ = parent;
	return std::move(dup);
}

void ClearFlag::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor*) {
	if (collected_) {
		return;
	}
	if (parent_->tangible_) {
		bool prev_active = active_;
		active_ = false;
		if (auto* above = map->view(pos_above())) {
			if (real_) {
				if (is_player_rep(above)) {
					active_ = true;
				}
			} else {
				if (auto* mod = above->modifier()) {
					switch (mod->mod_code()) {
					case ModCode::AutoBlock:
					case ModCode::PuppetBlock:
						active_ = true;
						break;
					}
				}
			}
		}
		if (active_ != prev_active) {
			delta_frame->push(std::make_unique<ClearFlagToggleDelta>(this));
			map->clear_flags_changed_ = true;
		}
	}
}

void ClearFlag::setup_on_put(RoomMap* map, DeltaFrame*, bool real) {
	map->clear_flags_.push_back(this);
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
	if (real && map->global_ && map->global_->has_flag(get_clear_flag_code(map->zone_))) {
		collected_ = true;
	}
}

void ClearFlag::cleanup_on_take(RoomMap* map, DeltaFrame*, bool real) {
	map->remove_clear_flag(this);
	map->remove_listener(this, pos_above());
}

void ClearFlag::destroy(MoveProcessor* mp, CauseOfDeath) {
	signal_animation(mp->anims_, mp->delta_frame_);
}

void ClearFlag::signal_animation(AnimationManager* anims, DeltaFrame* delta_frame) {
	if (parent_->tangible_) {
		anims->receive_signal(AnimationSignal::FlagOn, parent_, delta_frame);
	} else {
		anims->receive_signal(AnimationSignal::FlagOff, parent_, delta_frame);
	}
}

int ClearFlag::color() {
	int color = GOLD;
	if (collected_) {
		color = RED;
	} else if (active_) {
		if (real_) {
			color = GREEN;
		} else {
			color = DARK_PURPLE;
		}
	} else if (!real_) {
		color = PURPLE;
	}
	return color;
}

void ClearFlag::draw(GraphicsManager* gfx, FPoint3 p) {
	gfx->flag.push_instance(glm::vec3(p.x, p.y, p.z + 1.0f), glm::vec3(1.0f), BlockTexture::Blank, color());
}


ClearFlagToggleDelta::ClearFlagToggleDelta(ClearFlag* flag) :
	flag_{ flag } {}

ClearFlagToggleDelta::ClearFlagToggleDelta(FrozenObject flag) :
	flag_{ flag } {}

ClearFlagToggleDelta::~ClearFlagToggleDelta() {}

void ClearFlagToggleDelta::serialize(MapFileO& file) {
	flag_.serialize(file);
}

void ClearFlagToggleDelta::revert(RoomMap* room_map) {
	auto* flag = static_cast<ClearFlag*>(flag_.resolve_mod(room_map));
	flag->active_ = !flag->active_;
}

DeltaCode ClearFlagToggleDelta::code() {
	return DeltaCode::ClearFlagToggleDelta;
}

std::unique_ptr<Delta> ClearFlagToggleDelta::deserialize(MapFileI& file) {
	return std::make_unique<ClearFlagToggleDelta>(file.read_frozen_obj());
}
