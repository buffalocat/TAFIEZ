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
	// Initialize frame buffer to hold all renders
	glGenFramebuffers(1, &fbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	// Make a color texture
	glGenTextures(1, &color_tex_);
	glBindTexture(GL_TEXTURE_2D, color_tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1200, 900, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// Make a depth/stencil buffer
	glGenRenderbuffers(1, &rbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1200, 900);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// Attach color and depth/stencil
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex_, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Make the VAO for post processing
	glGenVertexArrays(1, &screen_vao_);
	glBindVertexArray(screen_vao_);
	glGenBuffers(1, &screen_vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_);
	const float quad_vertex_data[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_data), quad_vertex_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
}

GraphicsManager::~GraphicsManager() {
	glDeleteBuffers(1, &fbo_);
	glDeleteTextures(1, &atlas_);
	glDeleteTextures(1, &color_tex_);
	glDeleteRenderbuffers(1, &rbo_);
	glDeleteVertexArrays(1, &screen_vao_);
	glDeleteBuffers(1, &screen_vbo_);
}

const int FADE_IN_FRAMES = 20;
const int FADE_OUT_FRAMES = 20;

void GraphicsManager::set_state(GraphicsState state) {
	state_ = state;
	switch (state) {
	case GraphicsState::None:
		post_shader_.setFloat("shadow", 1);
		break;
	case GraphicsState::FadeOut:
		state_counter_ = FADE_OUT_FRAMES;
		post_shader_.setFloat("shadow", 1);
		break;
	case GraphicsState::Black:
		post_shader_.setFloat("shadow", 0);
		break;
	case GraphicsState::FadeIn:
		state_counter_ = 0;
		post_shader_.setFloat("shadow", 0);
		break;
	}
}

void GraphicsManager::update() {
	switch (state_) {
	case GraphicsState::FadeIn:
		post_shader_.setFloat("shadow", (double)(state_counter_) / FADE_IN_FRAMES);
		if (state_counter_ < FADE_IN_FRAMES) {
			++state_counter_;
		} else {
			state_ = GraphicsState::None;
		}
		break;
	case GraphicsState::FadeOut:
		post_shader_.setFloat("shadow", (double)(state_counter_) / FADE_OUT_FRAMES);
		if (state_counter_ > 0) {
			--state_counter_;
		} else {
			state_ = GraphicsState::Black;
		}
		break;
	}
}

bool GraphicsManager::in_animation() {
	switch (state_) {
	case GraphicsState::FadeIn:
	case GraphicsState::FadeOut:
		return true;
	case GraphicsState::None:
	case GraphicsState::Black:
	default:
		return false;
	}
}

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
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glClearColor(0.0f, 0.7f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GraphicsManager::post_rendering() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	post_shader_.use();
	glBindVertexArray(screen_vao_);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, color_tex_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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

void GraphicsManager::draw_objects() {
	cube.draw();
	top_cube.draw();
	six_squares.draw();

	diamond.draw();
	top_diamond.draw();
	six_squares_diamond.draw();

	cube_edges.draw();

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
	if (auto error = glGetError()) {
		std::cout << "Error detected after draw! " << error << std::endl;
	}
}

void TextRenderer::toggle_string_drawer(StringDrawer* drawer, bool active) {
	if (active) {
		string_drawers_.push_back(ProtectedStringDrawer(drawer));
	} else {
		drawer->kill_instance();
	}
}