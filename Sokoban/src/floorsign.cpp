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
	ObjectModifier(parent), content_{ content }, showing_text_{ showing } {}

FloorSign::~FloorSign() {}

std::string FloorSign::name() {
	return "FloorSign";
}

ModCode FloorSign::mod_code() {
	return ModCode::FloorSign;
}

void FloorSign::serialize(MapFileO& file) {
	file.write_long_str(content_.c_str(), (unsigned int)content_.size());
	file << showing_text_;
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
	bool should_display_text = false;
	if (parent_->tangible_) {
		if (auto* above = map->view(pos_above())) {
			if (dynamic_cast<Player*>(above)) {
				should_display_text = true;
			} else if (auto* player = dynamic_cast<Player*>(map->view(pos() + Point3{ 0,0,2 }))) {
				if (player->car_riding()) {
					should_display_text = true;
				}
			}
		}
	}
	if (showing_text_ != should_display_text) {
		toggle_displaying_text(should_display_text, map->gfx_, delta_frame);
	}
}

void FloorSign::toggle_displaying_text(bool should_display_text, GraphicsManager* gfx, DeltaFrame* delta_frame) {
	showing_text_ = should_display_text;
	if (should_display_text) {
		auto drawer = std::make_unique<SignTextDrawer>(
			gfx->fonts_->get_font(Fonts::KALAM_BOLD, 36),
			glm::vec4(0.9, 0.7, 0.8, 1.0), content_, 0.5f);
		drawer_instance_ = drawer.get();
		drawer_instance_->own_self(std::move(drawer));
	}
	gfx->toggle_string_drawer(drawer_instance_, should_display_text);
	if (delta_frame) {
		delta_frame->push(std::make_unique<SignToggleDelta>(this, gfx, !should_display_text));
	}
}

void FloorSign::setup_on_put(RoomMap* map, bool real) {
	map->add_listener(this, pos_above());
	map->activate_listener_of(this);
}

void FloorSign::cleanup_on_take(RoomMap* map, bool real) {
	map->remove_listener(this, pos_above());
}

void FloorSign::draw(GraphicsManager* gfx, FPoint3 p) {
	gfx->top_cube.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.9f, 0.9f, 0.1f), BlockTexture::Sign, glm::vec4(0.6, 0.3, 0.4, 1.0));
}