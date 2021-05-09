#include "stdafx.h"
#include "permanentswitch.h"

#include "delta.h"
#include "graphicsmanager.h"
#include "gameobject.h"
#include "roommap.h"
#include "mapfile.h"
#include "savefile.h"
#include "texture_constants.h"
#include "room.h"


PermanentSwitch::PermanentSwitch(GameObject* parent, int color, bool active, unsigned int global_id) :
	PressSwitch(parent, color, true, active), global_id_{ global_id } {}

PermanentSwitch::~PermanentSwitch() {}

void PermanentSwitch::make_str(std::string& str) {
	str += "PermanentSwitch";
	Switch::make_str(str);
}

ModCode PermanentSwitch::mod_code() {
	return ModCode::PermanentSwitch;
}

void PermanentSwitch::serialize(MapFileO& file) {
	file << color_ << active_;
	file.write_uint32(global_id_);
}

void PermanentSwitch::deserialize(MapFileI& file, GameObjectArray*, GameObject* parent) {
	int color;
	bool active;
	file >> color >> active;
	unsigned int global_id = file.read_uint32();
	parent->set_modifier(std::make_unique<PermanentSwitch>(parent, color, active, global_id));
}

bool PermanentSwitch::relation_check() {
	return true;
}

// This lets the editor keep global flags and room names up to date
void PermanentSwitch::relation_serialize(MapFileO& file) {
	file << MapCode::GlobalFlag;
	file.write_uint32(global_id_);
}

bool PermanentSwitch::check_send_signal(RoomMap* map, DeltaFrame* delta_frame) {
	if (active_) {
		return false;
	}
	if (should_toggle(map) || map->global_->has_flag(global_id_)) {
		toggle();
		delta_frame->push(std::make_unique<SwitchToggleDelta>(this));
		map->global_->add_flag_delta(global_id_, delta_frame);
		return true;
	}
	return false;
}

void PermanentSwitch::setup_on_editor_creation(EditorGlobalData* global, Room* room) {
	global_id_ = global->generate_flag();
	global->assign_flag(global_id_, room->name());
}

void PermanentSwitch::cleanup_on_editor_destruction(EditorGlobalData* global) {
	global->destroy_flag(global_id_);
}

void PermanentSwitch::draw(GraphicsManager* gfx, FPoint3 p) {
	BlockTexture tex;
	if (active_) {
		tex = BlockTexture::PermSwitchDown;
	} else {
		tex = BlockTexture::PermSwitchUp;
	}
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.8f, 0.8f, 0.1f), tex, color_);
}

std::unique_ptr<ObjectModifier> PermanentSwitch::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<PermanentSwitch>(*this);
	dup->parent_ = parent;
	dup->connect_to_signalers();
	return std::move(dup);
}