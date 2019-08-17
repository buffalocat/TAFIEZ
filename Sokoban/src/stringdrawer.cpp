#include "stdafx.h"
#include "stringdrawer.h"
#include "fontmanager.h"
#include "color_constants.h"
#include "common_constants.h"

StringDrawer::StringDrawer(Font* font, glm::vec4 color,
	std::string label, float x, float y, float sx, float sy) :
	color_{ color }, shader_{ font->shader_ }, tex_{ font->tex_ } {
	font->generate_string_verts(label.c_str(), x, y, sx, sy, vertices_, &width_);
	glGenVertexArrays(1, &VAO_);
	glBindVertexArray(VAO_);
	glGenBuffers(1, &VBO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(TextVertex), vertices_.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, TexCoords));
}

StringDrawer::~StringDrawer() {
	glDeleteVertexArrays(1, &VAO_);
	glDeleteBuffers(1, &VBO_);
	if (alive_ptr_) {
		*alive_ptr_ = false;
	}
}

void StringDrawer::set_color(int color) {
	color_ = COLOR_VECTORS[color];
}

void StringDrawer::set_color(glm::vec4 color) {
	color_ = color;
}


void StringDrawer::render() {
	shader_->setVec4("color", color_);
	glBindVertexArray(VAO_);
	glBindTexture(GL_TEXTURE_2D, tex_);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices_.size());
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


const unsigned int SIGN_FADE_FRAMES = 4;

SignTextDrawer::SignTextDrawer(Font* font, glm::vec4 color, std::string label, float y) :
	StringDrawer(font, color, label, 0, y, 1, 1) {}

SignTextDrawer::~SignTextDrawer() {}

void SignTextDrawer::own_self(std::unique_ptr<StringDrawer> self) {
	self_ = std::move(self);
}

void SignTextDrawer::update() {
	if (prepare_to_kill_) {
		if (--fade_counter_ == 0) {
			active_ = false;
		}
	} else {
		if (fade_counter_ < SIGN_FADE_FRAMES) {
			++fade_counter_;
		}
	}
	color_.w = (float)fade_counter_ / (float)SIGN_FADE_FRAMES;
}

void SignTextDrawer::kill_instance() {
	prepare_to_kill_ = true;
}

void SignTextDrawer::cleanup() {
	self_ = {};
}

const unsigned int ROOM_LABEL_DISPLAY_FRAMES = 180;
const unsigned int ROOM_LABEL_FADE_FRAMES = 20;

RoomLabelDrawer::RoomLabelDrawer(Font* font, glm::vec4 color, std::string label, float y) :
	StringDrawer(font, color, label, 0, y, 1, 1),
	lifetime_{ ROOM_LABEL_DISPLAY_FRAMES } {}

RoomLabelDrawer::~RoomLabelDrawer() {}

void RoomLabelDrawer::init() {
	lifetime_ = ROOM_LABEL_DISPLAY_FRAMES;
}

void RoomLabelDrawer::update() {
	if (lifetime_) {
		--lifetime_;
		if (lifetime_ > ROOM_LABEL_DISPLAY_FRAMES - ROOM_LABEL_FADE_FRAMES) {
			color_.w = (float)(ROOM_LABEL_DISPLAY_FRAMES - lifetime_) / (float)ROOM_LABEL_FADE_FRAMES;
		} else if (lifetime_ < ROOM_LABEL_FADE_FRAMES) {
			color_.w = (float)lifetime_ / (float)ROOM_LABEL_FADE_FRAMES;
		} else {
			color_.w = 1;
		}
	}
}