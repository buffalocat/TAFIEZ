#include "stdafx.h"
#include "autosavepanel.h"

#include "player.h"
#include "roommap.h"
#include "mapfile.h"
#include "moveprocessor.h"
#include "playingstate.h"
#include "savefile.h"
#include "texture_constants.h"
#include "graphicsmanager.h"

AutosavePanel::AutosavePanel(GameObject* parent, std::string label, bool active):
	ObjectModifier(parent), label_{ label }, active_{ active } {}

AutosavePanel::~AutosavePanel() {}

void AutosavePanel::make_str(std::string& str) {
	char buf[256];
	snprintf(buf, 256, "AutosavePanel(%s)", label_.c_str());
	str += buf;
}


ModCode AutosavePanel::mod_code() {
	return ModCode::AutosavePanel;
}


void AutosavePanel::serialize(MapFileO& file) {
  file.write_long_str(label_);
  file << active_;
}


void AutosavePanel::deserialize(MapFileI& file, GameObjectArray*, GameObject* parent) {
	auto label = file.read_long_str();
	bool active = file.read_byte();
	parent->set_modifier(std::make_unique<AutosavePanel>(parent, label, active));
}


void AutosavePanel::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (parent_->tangible_ && !active_) {
		if (auto* above = map->view(pos_above())) {
			if (is_player_rep(above)) {
				active_ = true;
				if (delta_frame) {
					delta_frame->push(std::make_unique<AutosavePanelDelta>(this));
				}
				mp->playing_state_->queued_autosave_ = this;
			}
		}
	}
}


void AutosavePanel::setup_on_put(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
}


void AutosavePanel::cleanup_on_take(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	map->remove_listener(this, pos_above());
}


void AutosavePanel::draw(GraphicsManager* gfx, FPoint3 p) {
	auto tex = active_ ? BlockTexture::AutosaveUsed : BlockTexture::AutosaveFresh;
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.9f, 0.9f, 0.1f), tex, glm::vec4(1.0f));
}

std::unique_ptr<ObjectModifier> AutosavePanel::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<AutosavePanel>(*this);
	dup->parent_ = parent;
	return std::move(dup);
}


AutosavePanelDelta::AutosavePanelDelta(AutosavePanel* panel) : Delta(), panel_{ panel } {}

AutosavePanelDelta::AutosavePanelDelta(FrozenObject panel) : Delta(), panel_{ panel } {}

AutosavePanelDelta::~AutosavePanelDelta() {}

void AutosavePanelDelta::serialize(MapFileO& file) {
	panel_.serialize(file);
}

void AutosavePanelDelta::revert(RoomMap* room_map) {
	static_cast<AutosavePanel*>(panel_.resolve_mod(room_map))->active_ = false;
}

DeltaCode AutosavePanelDelta::code() {
	return DeltaCode::AutosavePanelDelta;
}

std::unique_ptr<Delta> AutosavePanelDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<AutosavePanelDelta>(file.read_frozen_obj());
}

