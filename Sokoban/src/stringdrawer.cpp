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
	shader_.setVec4("color", color_);
	glBindVertexArray(VAO_);
	glBindTexture(GL_TEXTURE_2D, tex_);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices_.size());
}

void StringDrawer::update() {}

void StringDrawer::set_alive_ptr(bool* b_ptr) {
	alive_ptr_ = b_ptr;
}

void StringDrawer::kill_instance() {
	if (alive_ptr_) {
		*alive_ptr_ = false;
	}
	alive_ptr_ = nullptr;
}

RoomLabelDrawer::RoomLabelDrawer(Font* font, glm::vec4 color, std::string label, float y) :
	StringDrawer(font, color, label, 0, y, 1, 1),
	cooldown_{ AREA_NAME_DISPLAY_FRAMES } {}

RoomLabelDrawer::~RoomLabelDrawer() {}

void RoomLabelDrawer::init() {
	cooldown_ = AREA_NAME_DISPLAY_FRAMES;
}

void RoomLabelDrawer::update() {
	if (cooldown_) {
		--cooldown_;
		if (cooldown_ > AREA_NAME_DISPLAY_FRAMES - AREA_NAME_FADE_FRAMES) {
			color_.w = (float)(AREA_NAME_DISPLAY_FRAMES - cooldown_) / (float)AREA_NAME_FADE_FRAMES;
		} else if (cooldown_ < AREA_NAME_FADE_FRAMES) {
			color_.w = (float)cooldown_ / (float)AREA_NAME_FADE_FRAMES;
		} else {
			color_.w = 1;
		}
	}
}