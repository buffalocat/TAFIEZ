#include "stdafx.h"
#include "floorsign.h"

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
	file.write_long_str(content_.c_str(), (unsigned int)content_.size());
	file << active_;
}

void FloorSign::deserialize(MapFileI& file, RoomMap* map, GameObject* parent) {
	std::string content = file.read_long_str();
	bool showing_text = file.read_byte();
	auto sign = std::make_unique<FloorSign>(parent, content, showing_text);
	parent->set_modifier(std::move(sign));
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
		if (dynamic_cast<Player*>(above)) {
			should_display_text = true;
		} else if (auto* player = dynamic_cast<Player*>(map->view(pos() + Point3{ 0,0,2 }))) {
			if (player->car_riding()) {
				should_display_text = true;
			}
		}
	}
	if (active_ != should_display_text) {
		toggle_active(map->text_renderer(), delta_frame);
	}
}

void FloorSign::toggle_active(TextRenderer* text, DeltaFrame* delta_frame) {
	active_ = !active_;
	set_text_state(active_, text);
	if (delta_frame) {
		delta_frame->push(std::make_unique<SignToggleDelta>(this, text));
	}
}

void FloorSign::set_text_state(bool state, TextRenderer* text) {
	if (state) {
		auto drawer = std::make_unique<SignTextDrawer>(
			text->fonts_->get_font(Fonts::KALAM_BOLD, 36),
			glm::vec4(0.9, 0.7, 0.8, 1.0), content_, 0.5f);
		drawer_instance_ = drawer.get();
		drawer_instance_->own_self(std::move(drawer));
		text->string_drawers_.push_back(ProtectedStringDrawer(drawer_instance_));
	} else {
		drawer_instance_->kill_instance();
	}
}

void FloorSign::setup_on_put(RoomMap* map, bool real) {
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
	if (real && active_) {
		set_text_state(true, map->text_renderer());
	}
}

void FloorSign::cleanup_on_take(RoomMap* map, bool real) {
	map->remove_listener(this, pos_above());
	if (real && active_) {
		set_text_state(false, map->text_renderer());
	}
}

void FloorSign::draw(GraphicsManager* gfx, FPoint3 p) {
	gfx->top_cube.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.9f, 0.9f, 0.1f), BlockTexture::Sign, glm::vec4(0.6, 0.3, 0.4, 1.0));
}