#include "stdafx.h"
#include "clearflag.h"

#include "gameobject.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "texture_constants.h"
#include "roommap.h"
#include "player.h"
#include "delta.h"

ClearFlag::ClearFlag(GameObject* parent, int count, bool real, bool active, bool collected, char zone) :
	ObjectModifier(parent),
	count_{ count }, real_{ real }, active_{ active }, collected_{ collected }, zone_{ zone } {}

ClearFlag::~ClearFlag() {}

void ClearFlag::make_str(std::string& str) {
	char buf[32];
	snprintf(buf, 32, "ClearFlag:%s:%d", real_ ? "Real" : "Fake", count_);
	str += buf;
}

ModCode ClearFlag::mod_code() {
	return ModCode::ClearFlag;
}

void ClearFlag::serialize(MapFileO& file) {
	file << real_;
	file << active_;
}

void ClearFlag::deserialize(MapFileI& file, RoomMap* map, GameObject* parent) {
	bool real, active;
	file >> real >> active;
	bool collected = map->clear_flag_req_ == 0;
	auto cf = std::make_unique<ClearFlag>(parent, map->clear_flag_req_, real, active, collected, map->zone_);
	map->clear_flags_[cf.get()] = active;
	parent->set_modifier(std::move(cf));
}

std::unique_ptr<ObjectModifier> ClearFlag::duplicate(GameObject* parent, RoomMap* map, DeltaFrame*) {
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
				if (dynamic_cast<Player*>(above)) {
					active_ = true;
				} else if (auto* player = dynamic_cast<Player*>(map->view(pos() + Point3{ 0, 0, 2 }))) {
					if (player->car_riding()) {
						active_ = true;
					}
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
			map->clear_flags_[this] = active_;
			delta_frame->push(std::make_unique<ClearFlagToggleDelta>(this, map));
			map->clear_flags_changed_ = true;
		}
	}
}

void ClearFlag::setup_on_put(RoomMap* map, bool real) {
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
}

void ClearFlag::cleanup_on_take(RoomMap* map, bool real) {
	map->remove_listener(this, pos_above());
}

void ClearFlag::draw(GraphicsManager* gfx, FPoint3 p) {
	int color = GOLD;
	if (collected_) {
		color = DARK_RED;
	} else if (active_) {
		if (real_) {
			color = GREEN;
		} else {
			color = DARK_PURPLE;
		}
	} else if (!real_) {
		color = PURPLE;
	}
	gfx->flag.push_instance(glm::vec3(p.x, p.y, p.z + 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), BlockTexture::Blank, color);
}
