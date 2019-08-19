#include "stdafx.h"
#include "graphicsmanager.h"
#include "common_constants.h"
#include "texture_constants.h"
#include "fontmanager.h"
#include "stringdrawer.h"
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

ProtectedStringDrawer::ProtectedStringDrawer(StringDrawer* drawer) : drawer_{ drawer }, alive_{ true } {
	drawer_->set_alive_ptr(&alive_);
}

ProtectedStringDrawer::~ProtectedStringDrawer() {
	if (alive_) {
		drawer_->set_alive_ptr(nullptr);
	}
}

ProtectedStringDrawer::ProtectedStringDrawer(ProtectedStringDrawer&& p) : drawer_{ p.drawer_ }, alive_{ p.alive_ } {
	p.alive_ = false;
	p.drawer_ = nullptr;
	if (alive_) {
		drawer_->set_alive_ptr(&alive_);
	}
}

GraphicsManager::GraphicsManager(GLFWwindow* window) :
	window_{ window } {
	fonts_ = std::make_unique<FontManager>(&text_shader_);
	instanced_shader_.use();
	instanced_shader_.setFloat("lightMixFactor", 0.7);
	load_texture_atlas();
	instanced_shader_.setFloat("texScale", 1.0f / TEXTURE_ATLAS_SIZE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GraphicsManager::~GraphicsManager() {}

GLFWwindow* GraphicsManager::window() {
	return window_;
}

void GraphicsManager::load_texture_atlas() {
	glGenTextures(1, &atlas_);
	glBindTexture(GL_TEXTURE_2D, atlas_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, channels;
	unsigned char *texture_data = stbi_load("resources/textures.png", &width, &height, &channels, STBI_rgb_alpha);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	stbi_image_free(texture_data);
}

void GraphicsManager::prepare_object_rendering() {
	instanced_shader_.use();
	glBindTexture(GL_TEXTURE_2D, atlas_);
}

void GraphicsManager::set_PV(glm::mat4 PV) {
	if (PV != PV_) {
		PV_ = PV;
		instanced_shader_.setMat4("PV", PV);
	}
}

void GraphicsManager::set_light_source(glm::vec3 dir) {
	instanced_shader_.setVec3("lightSource", dir);
}

void GraphicsManager::draw() {
	cube.draw();
	top_cube.draw();
	six_squares.draw();

	diamond.draw();
	top_diamond.draw();
	six_squares_diamond.draw();

	wall.draw();
}

TextRenderer::TextRenderer(FontManager* fonts) :
	fonts_{ fonts }, text_shader_{ fonts->text_shader_ } {}

// These StringDrawers are self-owning, so we have to kill them here
TextRenderer::~TextRenderer() {
	for (auto& p : string_drawers_) {
		if (p.alive_) {
			p.drawer_->cleanup();
		}
	}
}

void TextRenderer::draw() {
	text_shader_->use();
	std::vector<ProtectedStringDrawer> new_drawers{};
	for (auto& p : string_drawers_) {
		if (p.alive_) {
			if (p.drawer_->active_) {
				p.drawer_->update();
				p.drawer_->render();
				new_drawers.push_back(std::move(p));
			} else {
				p.drawer_->cleanup();
			}
		}
	}
	string_drawers_ = std::move(new_drawers);
}

void TextRenderer::toggle_string_drawer(StringDrawer* drawer, bool active) {
	if (active) {
		string_drawers_.push_back(ProtectedStringDrawer(drawer));
	} else {
		drawer->kill_instance();
	}
}