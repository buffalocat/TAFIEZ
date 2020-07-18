#include "stdafx.h"
#include "stringdrawer.h"
#include "fontmanager.h"
#include "graphicsmanager.h"
#include "color_constants.h"
#include "common_constants.h"
#include "texture_constants.h"

const float ZONE_STRING_HEIGHT = 0.98f;
const float LEVEL_STRING_HEIGHT = 0.65f;
const float SIGN_STRING_HEIGHT = 0.4f;
const float DEATH_STRING_HEIGHT = 0.25f;
const float DEATH_SUBSTRING_HEIGHT = -0.04f;

const float ZONE_STRING_BG_OPACITY = 0.7f;
const float LEVEL_STRING_BG_OPACITY = 0.7f;
const float SIGN_STRING_BG_OPACITY = 0.85f;

const int FLOOR_SIGN_FADE_FRAMES = 12;
const int DEATH_STRING_FADE_FRAMES = 4;


StringDrawer::StringDrawer(Font* font, glm::vec4 color,
	std::string label, float x, float y, float sx, float sy, float bg) :
	color_{ color }, bg_color_{ glm::vec4(0.6f,0.6f,0.6f,1.0f) }, opacity_{ 1.0f },
	shader_{ font->shader_ }, tex_{ font->tex_ }, bg_{ bg } {
	font->generate_string_verts(label.c_str(), x, y, sx, sy, vertices_, &width_, &height_);
	glGenVertexArrays(1, &VAO_);
	glBindVertexArray(VAO_);
	glGenBuffers(1, &VBO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(TextVertex), vertices_.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, TexCoords));
	if (bg > 0 && !label.empty()) {
		generate_bg_verts(x, y, font->font_size_ * 2.0f / SCREEN_HEIGHT);
		glGenVertexArrays(1, &bg_VAO_);
		glBindVertexArray(bg_VAO_);
		glGenBuffers(1, &bg_VBO_);
		bind_bg_vbo();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, Position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, TexCoords));
	}
}

StringDrawer::~StringDrawer() {
	glDeleteVertexArrays(1, &VAO_);
	glDeleteBuffers(1, &VBO_);
	if (alive_ptr_) {
		*alive_ptr_ = false;
	}
}

void StringDrawer::bind_bg_vbo() {
	glBindBuffer(GL_ARRAY_BUFFER, bg_VBO_);
	glBufferData(GL_ARRAY_BUFFER, bg_vertices_.size() * sizeof(TextVertex), bg_vertices_.data(), GL_STATIC_DRAW);
}

const float FONT_TOP_FACTOR = -0.3f;
const float FONT_BOTTOM_FACTOR = 0.08f;
const float TB_PAD_TOP = 0.04f;
const float TB_PAD_BOTTOM = 0.04f;
const float TB_PAD_SIDE = 0.03f;

void StringDrawer::generate_bg_verts(float x, float y, float font_height) {
	float xa = x - width_ / 2;
	float xb = x + width_ / 2;
	float ya = y + FONT_TOP_FACTOR * font_height;
	float yb = y - height_ - FONT_BOTTOM_FACTOR * font_height;
	float key_x[] = { xa - TB_PAD_SIDE, xa, xb, xb + TB_PAD_SIDE };
	float key_y[] = { ya + TB_PAD_TOP, ya, yb, yb - TB_PAD_BOTTOM };
	ParticleTexture tb_tex[] = {
		ParticleTexture::TextboxTL,
		ParticleTexture::TextboxL,
		ParticleTexture::TextboxBL,
		ParticleTexture::TextboxT,
		ParticleTexture::SolidSquare,
		ParticleTexture::TextboxB,
		ParticleTexture::TextboxTR,
		ParticleTexture::TextboxR,
		ParticleTexture::TextboxBR,
	};
	bg_vertices_.reserve(9 * 6);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			glm::vec2 tex = tex_to_vec(tb_tex[3 * i + j]);
			float u = tex.x;
			float v = tex.y;
			TextVertex box[4] = {
				TextVertex{{key_x[i], key_y[j]}, {u, v}},
				TextVertex{{key_x[i+1], key_y[j]}, {u + 1, v}},
				TextVertex{{key_x[i], key_y[j+1]}, {u, v + 1}},
				TextVertex{{key_x[i+1], key_y[j+1]}, {u + 1, v + 1}},
			};
			for (int i : {0, 1, 2, 2, 1, 3}) {
				bg_vertices_.push_back(box[i]);
			}
		}
	}
}

void StringDrawer::set_color(int color) {
	color_ = COLOR_VECTORS[color];
}

void StringDrawer::set_color(glm::vec4 color) {
	color_ = color;
}

void StringDrawer::render_bg(Shader* shader) {
	if (bg_ > 0 && opacity_ > 0) {
		glm::vec4 color = bg_color_;
		color.w *= bg_ * std::min(2 * opacity_, 1.0f);
		shader->setVec4("color", color);
		glBindVertexArray(bg_VAO_);
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)bg_vertices_.size());
	}
}

void StringDrawer::render() {
	if (opacity_ > 0) {
		glm::vec4 color = color_;
		color.w = opacity_;
		shader_->setVec4("color", color);
		glBindVertexArray(VAO_);
		glBindTexture(GL_TEXTURE_2D, tex_);
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices_.size());
	}
}

void StringDrawer::update() {}

void StringDrawer::cleanup() {}

void StringDrawer::set_alive_ptr(bool* b_ptr) {
	alive_ptr_ = b_ptr;
}

void StringDrawer::kill_instance() {
	if (alive_ptr_) {
		*alive_ptr_ = false;
	}
	alive_ptr_ = nullptr;
}


IndependentStringDrawer::IndependentStringDrawer(Font* font, glm::vec4 color, std::string label, float y, int fade_frames, float bg) :
	StringDrawer(font, color, label, 0, y, 1, 1, bg), fade_frames_{ fade_frames } {}

IndependentStringDrawer::~IndependentStringDrawer() {}

void IndependentStringDrawer::own_self(std::unique_ptr<StringDrawer> self) {
	self_ = std::move(self);
}

void IndependentStringDrawer::update() {
	if (prepare_to_kill_) {
		if (--fade_counter_ == 0) {
			active_ = false;
		}
	} else {
		if (fade_counter_ < fade_frames_) {
			++fade_counter_;
		}
	}
	opacity_ = (float)fade_counter_ / (float)fade_frames_;
}

void IndependentStringDrawer::kill_instance() {
	prepare_to_kill_ = true;
}

void IndependentStringDrawer::cleanup() {
	self_.reset(nullptr);
}

const unsigned int ROOM_LABEL_DISPLAY_FRAMES = 180;
const unsigned int ROOM_LABEL_FADE_FRAMES = 20;

RoomLabelDrawer::RoomLabelDrawer(Font* font, glm::vec4 color, std::string label, float y, float bg, bool cam_icon) :
	StringDrawer(font, color, label, 0, y, 1, 1, bg),
	lifetime_{ ROOM_LABEL_DISPLAY_FRAMES } {
	if (cam_icon) {
		generate_cam_icon_verts();
	}
}

RoomLabelDrawer::~RoomLabelDrawer() {}

void RoomLabelDrawer::init() {
	lifetime_ = ROOM_LABEL_DISPLAY_FRAMES;
}

void RoomLabelDrawer::force_fade() {
	if (lifetime_ > ROOM_LABEL_FADE_FRAMES) {
		lifetime_ = ROOM_LABEL_FADE_FRAMES;
	}
}

void RoomLabelDrawer::update() {
	if (lifetime_) {
		--lifetime_;
		if (lifetime_ > ROOM_LABEL_DISPLAY_FRAMES - ROOM_LABEL_FADE_FRAMES) {
			opacity_ = (float)(ROOM_LABEL_DISPLAY_FRAMES - lifetime_) / (float)ROOM_LABEL_FADE_FRAMES;
		} else if (lifetime_ < ROOM_LABEL_FADE_FRAMES) {
			opacity_ = (float)lifetime_ / (float)ROOM_LABEL_FADE_FRAMES;
		} else {
			opacity_ = 1;
		}
	}
}

void RoomLabelDrawer::generate_cam_icon_verts() {
	glm::vec2 tex = tex_to_vec(ParticleTexture::Camera);
	float u = tex.x;
	float v = tex.y;
	float x = -0.985f;
	float y = 0.98f;
	float h = 0.2f;
	float w = 0.15f;
	TextVertex box[4] = {
		TextVertex{{x, y}, {u, v}},
		TextVertex{{x + w, y}, {u + 1, v}},
		TextVertex{{x, y - h}, {u, v + 1}},
		TextVertex{{x + w, y - h}, {u + 1, v + 1}},
	};
	for (int i : {0, 1, 2, 2, 1, 3}) {
		bg_vertices_.push_back(box[i]);
	}
	bind_bg_vbo();
}