#include "stdafx.h"
#include "floorsign.h"

#include "moveprocessor.h"
#include "playingstate.h"
#include "savefile.h"
#include "fontmanager.h"
#include "stringdrawer.h"
#include "graphicsmanager.h"
#include "texture_constants.h"
#include "mapfile.h"
#include "common_constants.h"
#include "roommap.h"
#include "player.h"
#include "delta.h"

FloorSign::FloorSign(GameObject* parent, std::string content, bool showing) :
	ObjectModifier(parent), content_{ content }, active_{ showing } {}

FloorSign::~FloorSign() {}

void FloorSign::make_str(std::string& str) {
	char buf[256];
	snprintf(buf, 256, "FloorSign:\"%s\"", content_.c_str());
	str += buf;
}

ModCode FloorSign::mod_code() {
	return ModCode::FloorSign;
}

void FloorSign::serialize(MapFileO& file) {
	file.write_long_str(content_);
	file << active_;
}

void FloorSign::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	std::string content = file.read_long_str();
	bool showing_text = file.read_byte();
	auto sign = std::make_unique<FloorSign>(parent, content, showing_text);
	parent->set_modifier(std::move(sign));
}

bool FloorSign::relation_check() {
	return learn_flag_ != 0;
}

void FloorSign::relation_serialize(MapFileO& file) {
	file << MapCode::FloorSignFlag;
	file << pos();
	file.write_uint32(learn_flag_);
}

std::unique_ptr<ObjectModifier> FloorSign::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	return std::make_unique<FloorSign>(parent, content_, false);
}

void FloorSign::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (!parent_->tangible_) {
		return;
	}
	bool should_display_text = false;
	if (auto* above = map->view(pos_above())) {
		if (is_player_rep(above)) {
			should_display_text = true;
		}
	}
	if (active_ != should_display_text) {
		if (learn_flag_ != 0) {
			PlayingGlobalData* global = mp->playing_state_->global_;
			global->add_flag_delta(learn_flag_, delta_frame);
			delta_frame->push(std::make_unique<LearnFlagDelta>(this, learn_flag_));
			learn_flag_ = 0;
		}
		toggle_active(map->text_renderer(), delta_frame);
	}
}

void FloorSign::toggle_active(TextRenderer* text, DeltaFrame* delta_frame) {
	active_ = !active_;
	set_text_state(active_, text);
	if (delta_frame) {
		delta_frame->push(std::make_unique<SignToggleDelta>(this));
	}
}

void FloorSign::set_text_state(bool state, TextRenderer* text) {
	if (state) {
		auto drawer = std::make_unique<IndependentStringDrawer>(
			text->fonts_->get_font(Fonts::ABEEZEE, 36),
			COLOR_VECTORS[DARK_SALMON], content_, SIGN_STRING_HEIGHT, FLOOR_SIGN_FADE_FRAMES, SIGN_STRING_BG_OPACITY);
		drawer_instance_ = drawer.get();
		drawer_instance_->own_self(std::move(drawer));
		text->string_drawers_.push_back(ProtectedStringDrawer(drawer_instance_));
	} else {
		drawer_instance_->kill_instance();
	}
}

void FloorSign::setup_on_put(RoomMap* map, DeltaFrame*, bool real) {
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
	if (real && active_) {
		set_text_state(true, map->text_renderer());
	}
}

void FloorSign::cleanup_on_take(RoomMap* map, DeltaFrame*, bool real) {
	map->remove_listener(this, pos_above());
	if (real && active_) {
		set_text_state(false, map->text_renderer());
	}
}

void FloorSign::draw(GraphicsManager* gfx, FPoint3 p) {
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.9f, 0.9f, 0.1f), BlockTexture::Sign, DARK_SALMON);
}


SignToggleDelta::SignToggleDelta(FloorSign* sign) :
	Delta(), sign_{ sign } {}

SignToggleDelta::SignToggleDelta(FrozenObject sign) :
	Delta(), sign_{ sign } {}

SignToggleDelta::~SignToggleDelta() {}

void SignToggleDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	sign_.serialize(file, arr);
}

void SignToggleDelta::revert(RoomMap* room_map) {
	static_cast<FloorSign*>(sign_.resolve_mod(room_map))->toggle_active(room_map->text_renderer(), nullptr);
}

DeltaCode SignToggleDelta::code() {
	return DeltaCode::SignToggleDelta;
}

std::unique_ptr<Delta> SignToggleDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<SignToggleDelta>(file.read_frozen_obj());
}


LearnFlagDelta::LearnFlagDelta(FloorSign* sign, unsigned int flag) :
	Delta(), sign_{ sign }, flag_{ flag } {}

LearnFlagDelta::LearnFlagDelta(FrozenObject sign, unsigned int flag) :
	Delta(), sign_{ sign }, flag_{ flag } {}

LearnFlagDelta::~LearnFlagDelta() {}

void LearnFlagDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	sign_.serialize(file, arr);
	file.write_uint32(flag_);
}

void LearnFlagDelta::revert(RoomMap* room_map) {
	static_cast<FloorSign*>(sign_.resolve_mod(room_map))->learn_flag_ = flag_;
}

DeltaCode LearnFlagDelta::code() {
	return DeltaCode::LearnFlagDelta;
}

std::unique_ptr<Delta> LearnFlagDelta::deserialize(MapFileIwithObjs& file) {
	auto sign = file.read_frozen_obj();
	auto flag = file.read_uint32();
	return std::make_unique<LearnFlagDelta>(sign, flag);
}
