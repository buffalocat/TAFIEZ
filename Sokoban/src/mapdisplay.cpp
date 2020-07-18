#include "stdafx.h"
#include "mapdisplay.h"

#include "gameobject.h"

#include "savefile.h"
#include "playingstate.h"
#include "globalflagconstants.h"

#include "animationmanager.h"
#include "graphicsmanager.h"
#include "fontmanager.h"
#include "texture_constants.h"


MapDisplay::MapDisplay(GameObject* parent) : ObjectModifier(parent) {
	glGenVertexArrays(1, &VAO_);
	glGenBuffers(1, &VBO_);
}

MapDisplay::~MapDisplay() {
	glDeleteVertexArrays(1, &VAO_);
	glDeleteBuffers(1, &VBO_);
}

void MapDisplay::make_str(std::string& str) {
	str += "MapDisplay";
}

ModCode MapDisplay::mod_code() {
	return ModCode::MapDisplay;
}

void MapDisplay::deserialize(MapFileI& file, RoomMap* map, GameObject* parent) {
	auto fg = std::make_unique<MapDisplay>(parent);
	parent->set_modifier(std::move(fg));
}

std::unique_ptr<ObjectModifier> MapDisplay::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<MapDisplay>(*this);
	dup->parent_ = parent;
	return std::move(dup);
}

void MapDisplay::signal_animation(AnimationManager* anims, DeltaFrame*) {
	anims->receive_signal(AnimationSignal::MapDisplay, parent_, nullptr);
}


void MapDisplay::init_sprites(PlayingState* state) {
	state_ = state;
	global_ = state->global_;
	sprites_.clear();
	char_verts_.clear();
	font_ = state->text_->fonts_->get_font(Fonts::ABEEZEE, 72);
	parent_pos_ = glm::vec3(parent_->real_pos());
	generate_map();

	glBindVertexArray(VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, char_verts_.size() * sizeof(TextVertex3), char_verts_.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex3), (void*)offsetof(TextVertex3, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex3), (void*)offsetof(TextVertex3, TexCoords));
}


void MapDisplay::draw_special(GraphicsManager* gfx, GLuint atlas) {
	init_sprites(state_);
	for (auto& sprite : sprites_) {
		gfx->square_0.push_instance(sprite.pos, glm::vec3(0.6f), sprite.tex, glm::vec4(1.0f));
	}
	gfx->prepare_draw_objects_particle_atlas(atlas);
	gfx->draw_objects();
	gfx->text_shader_spacial_.use();
	gfx->text_shader_spacial_.setMat4("PV", gfx->PV_);
	gfx->text_shader_spacial_.setVec4("color", COLOR_VECTORS[BLACK]);
	glBindVertexArray(VAO_);
	glBindTexture(GL_TEXTURE_2D, font_->tex_);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)char_verts_.size());
}

void MapDisplay::generate_map() {
	pos_ = glm::vec3(0, 1.54f, 0) + parent_pos_;
	draw_zone('H', 0, 0);
	pos_ = glm::vec3(-4, 1.54f, -3) + parent_pos_;
	draw_zone('X', 0, 0);
	pos_ = glm::vec3(4, 1.54f, 5) + parent_pos_;
	draw_hub(HubCode::Omega, 0, 0);
}

bool MapDisplay::visited(char zone) {
	if (zone == 'X') {
		for (int i = 0; i < 4; ++i) {
			if (global_->has_flag(X_ALT_ACCESSED_GLOBAL_FLAGS[i])) {
				return true;
			}
			return false;
		}
	}
	return global_->has_flag(get_zone_access_code(zone));
}

void MapDisplay::draw_tex(ParticleTexture tex, float dx, float dy) {
	sprites_.push_back({ pos_ + glm::vec3(dx, -0.02, dy), tex });
}

void MapDisplay::draw_char(char c, float dx, float dy) {
	font_->generate_spacial_char_verts(c,
		pos_ + glm::vec3(dx, 0.02, dy),
		glm::vec3(1, 0, 0),
		glm::vec3(0, 0, 1),
		0.004f, char_verts_);
}

bool MapDisplay::draw_zone(char zone, float dx, float dy) {
	if (!visited(zone)) {
		return false;
	}
	
	// Draw connecting lines
	if (zone == 'D' && visited('O')) {
		draw_tex(ParticleTexture::TeeLine, 0, 1);
		draw_tex(ParticleTexture::VertLine, 0, 0.5);
		draw_tex(ParticleTexture::HorLine, -0.5, 1);
		dx = -1;
	} else if (zone == 'O' && visited('D')) {
		draw_tex(ParticleTexture::HorLine, 0.5, 1);
		dx = 1;
	} else if (dx != 0) {
		draw_tex(ParticleTexture::HorLine, dx/2, 0);
	} else if (dy != 0) {
		draw_tex(ParticleTexture::VertLine, 0, dy/2);
	}

	pos_.x += dx;
	pos_.z += dy;
	bool exits_done = true;

	// Deliberately non short circuiting '&' in this switch
	switch (zone) {
	case 'A':
	{
		exits_done = draw_zone('Q', 0, 1);
		break;
	}
	case 'B':
	{
		break;
	}
	case 'C':
	{
		exits_done = draw_zone('D', 0, 1) &
			draw_zone('O', 0, 1) &
			draw_zone('N', -1, 0) &
			draw_zone('0', 1, 0);
		break;
	}
	case 'D':
	{
		exits_done = draw_zone('5', -1, 0) &
			draw_warp(HubCode::Alpha, 'H', 0, 1);
		break;
	}
	case 'E':
	{
		exits_done = draw_zone('U', 0, 1) &
			draw_warp(HubCode::Beta, 'X', 1, 0) &
			draw_zone('6', 0, -1);
		break;
	}
	case 'F':
	{
		break;
	}
	case 'G':
	{
		exits_done = draw_zone('2', 0, 1) &
			draw_zone('7', -1, 0);
		break;
	}
	case 'H':
	{
		exits_done = draw_hub(HubCode::Alpha, 0, 2) &
			draw_hub(HubCode::Beta, 2, 0) &
			draw_hub(HubCode::Gamma, 0, -2) &
			draw_hub(HubCode::Delta, -2, 0);
		break;
	}
	case 'I':
	{
		break;
	}
	case 'J':
	{
		break;
	}
	case 'K':
	{
		exits_done = draw_zone('F', 0, -1);
		break;
	}
	case 'L':
	{
		exits_done = draw_warp(HubCode::Gamma, 'H', 0, 1) &
			draw_warp(HubCode::Gamma, 'X', -1, 0);
		break;
	}
	case 'M':
	{
		exits_done = draw_warp(HubCode::Delta, 'H', 0, -1);
		break;
	}
	case 'N':
	{
		break;
	}
	case 'O':
	{
		exits_done = draw_warp(HubCode::Alpha, 'X', 1, 0);
		break;
	}
	case 'P':
	{
		exits_done = draw_zone('M', -1, 0);
		break;
	}
	case 'Q':
	{
		exits_done = draw_warp(HubCode::Beta, 'H', 0, 1);
		break;
	}
	case 'R':
	{
		exits_done = draw_zone('I', -1, 0) &
			draw_warp(HubCode::Delta, 'X', 1, 0);
		break;
	}
	case 'S':
	{
		exits_done = draw_zone('4', -1, 0);
		break;
	}
	case 'T':
	{
		break;
	}
	case 'U':
	{
		exits_done = draw_zone('8', 0, 1);
		break;
	}
	case 'V':
	{
		exits_done = draw_zone('G', -1, 0);
		break;
	}
	case 'W':
	{
		exits_done = draw_zone('3', 1, 0);
		break;
	}
	case 'X':
	{
		break;
	}
	case 'Y':
	{
		exits_done = draw_zone('B', -1, 0);
		break;
	}
	case 'Z':
	{
		exits_done = draw_zone('J', 1, 0);
		break;
	}
	case '0':
	{
		break;
	}
	case '1':
	{
		break;
	}
	case '2':
	{
		break;
	}
	case '3':
	{
		exits_done = draw_warp(HubCode::Omega, 'H', 0, -1);
		break;
	}
	case '4':
	{
		break;
	}
	case '5':
	{
		break;
	}
	case '6':
	{
		exits_done = draw_zone('9', 0, -1);
		break;
	}
	case '7':
	{
		exits_done = draw_zone('T', 0, 1);
		break;
	}
	case '8':
	{
		break;
	}
	case '9':
	{
		break;
	}
	case '!':
	{
		break;
	}
	}
	ParticleTexture tex;
	if (global_->has_flag(get_clear_flag_code(zone))) {
		tex = exits_done ? ParticleTexture::GoldSolid : ParticleTexture::GoldDashed;
	} else {
		tex = exits_done ? ParticleTexture::GreySolid : ParticleTexture::GreyDashed;
	}
	sprites_.push_back({ pos_, tex });
	draw_char(zone, 0, 0);
	pos_.x -= dx;
	pos_.z -= dy;
	return true;
}

bool MapDisplay::draw_hub(HubCode hub, float dx, float dy) {
	if (!global_->has_flag(HUB_ACCESSED_GLOBAL_FLAGS[static_cast<int>(hub)])) {
		return false;
	}
	if (dx != 0) {
		for (int i = 1; i < 4; ++i) {
			draw_tex(ParticleTexture::HorLine, i * dx / 4, 0);
		}
	} else if (dy != 0) {
		for(int i = 1; i < 4; ++i) {
			draw_tex(ParticleTexture::VertLine, 0, i * dy / 4);
		}
	}

	pos_.x += dx;
	pos_.z += dy;
	ParticleTexture tex;
	switch (hub) {
	case HubCode::Alpha: {
		tex = ParticleTexture::Alpha;
		draw_zone('V', -1, 0);
		draw_zone('C', 0, 1);
		draw_zone('K', 1, 0);
		break;
	}
	case HubCode::Beta:
	{
		tex = ParticleTexture::Beta;
		draw_zone('A', 0, 1);
		draw_zone('E', 1, 0);
		draw_zone('S', 0, -1);
		break;
	}
	case HubCode::Gamma:
	{
		tex = ParticleTexture::Gamma;
		draw_zone('W', 1, 0);
		draw_zone('Z', 0, -1);
		draw_zone('L', -1, 0);
		break;
	}
	case HubCode::Delta:
	{
		tex = ParticleTexture::Delta;
		draw_zone('P', 0, -1);
		draw_zone('Y', -1, 0);
		draw_zone('R', 0, 1);
		break;
	}
	case HubCode::Omega:
	{
		tex = ParticleTexture::Omega;
		draw_zone('1', -1, 0);
		draw_zone('!', 0, -1);
		break;
	}
	}
	sprites_.push_back({ pos_, tex });
	pos_.x -= dx;
	pos_.z -= dy;
	return true;
}

bool MapDisplay::draw_warp(HubCode hub, char zone, float dx, float dy) {
	bool should_draw;
	if (zone == 'H') {
		should_draw = global_->has_flag(HUB_ALT_ACCESSED_GLOBAL_FLAGS[static_cast<int>(hub)]);
	} else { // zone == 'X'
		should_draw = global_->has_flag(X_ALT_ACCESSED_GLOBAL_FLAGS[static_cast<int>(hub)]);
	}
	if (should_draw) {
		sprites_.push_back({ pos_ + glm::vec3(dx, 0, dy), ParticleTexture::PinkSolid });
		if (dx != 0) {
			draw_tex(ParticleTexture::HorDashes, dx / 2, 0);
		} else if (dy != 0) {
			draw_tex(ParticleTexture::VertDashes, 0, dy / 2);
		}
		draw_char(zone, dx, dy);
	}
	return should_draw;
}
