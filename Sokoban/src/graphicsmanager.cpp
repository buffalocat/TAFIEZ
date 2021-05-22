#include "stdafx.h"
#include "graphicsmanager.h"
#include "common_constants.h"
#include "texture_constants.h"
#include "fontmanager.h"
#include "animationmanager.h"
#include "stringdrawer.h"
#include "model.h"
#include "window.h"

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

GraphicsManager::GraphicsManager(OpenGLWindow* window) :
	window_{ window } {
	fonts_ = std::make_unique<FontManager>(window_, &text_shader_);
	instanced_shader_.use();
	instanced_shader_.setFloat("lightMixFactor", 0.7);
	load_texture_atlas();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Make the VAO for post processing
	glGenVertexArrays(1, &screen_vao_);
	glBindVertexArray(screen_vao_);
	glGenBuffers(1, &screen_vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_);
	const float quad_vertex_data[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
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

void GraphicsManager::generate_framebuffer() {
	if (fbo_) {
		glDeleteFramebuffers(1, &fbo_);
	}
	if (color_tex_) {
		glDeleteTextures(1, &color_tex_);
	}
	if (rbo_) {
		glDeleteRenderbuffers(1, &rbo_);
	}
	// Initialize frame buffer to hold all renders
	glGenFramebuffers(1, &fbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	// Make a color texture
	glGenTextures(1, &color_tex_);
	glBindTexture(GL_TEXTURE_2D, color_tex_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_->viewport_size_[0], window_->viewport_size_[1],
		0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	// Make a depth/stencil buffer
	glGenRenderbuffers(1, &rbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_->viewport_size_[0], window_->viewport_size_[1]);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// Attach color and depth/stencil
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex_, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const int FADE_IN_FRAMES = 20;
const int FADE_OUT_FRAMES = 20;

void GraphicsManager::set_state(GraphicsState state) {
	state_ = state;
	switch (state) {
	case GraphicsState::None:
		shadow_ = 1;
		break;
	case GraphicsState::FadeOut:
		state_counter_ = FADE_OUT_FRAMES;
		shadow_ = 1;
		break;
	case GraphicsState::Black:
		shadow_ = 0;
		break;
	case GraphicsState::FadeIn:
		state_counter_ = 0;
		shadow_ = 0;
		break;
	}
}

void GraphicsManager::update() {
	// Handle state transitions
	switch (state_) {
	case GraphicsState::Black:
		shadow_ = 0.0f;
		break;
	case GraphicsState::None:
		shadow_ = 1.0f;
		break;
	case GraphicsState::FadeIn:
		shadow_ = (double)(state_counter_) / FADE_IN_FRAMES;
		if (state_counter_ < FADE_IN_FRAMES) {
			++state_counter_;
		} else {
			state_ = GraphicsState::None;
		}
		break;
	case GraphicsState::FadeOut:
		shadow_ = (double)(state_counter_) / FADE_IN_FRAMES;
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

void GraphicsManager::set_PV(glm::mat4 proj, glm::mat4 view) {
	proj_ = proj;
	view_ = view;
	PV_ = proj * view;
}

void GraphicsManager::set_light_source(glm::vec3 light_source) {
	light_source_ = light_source;
}

void GraphicsManager::clear_screen(glm::vec4 clear_color) {
	glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsManager::pre_object_rendering() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, window_->viewport_size_[0], window_->viewport_size_[1]);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void GraphicsManager::pre_particle_rendering() {
	particle_shader_.use();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	particle_shader_.setMat4("Proj", proj_);
	particle_shader_.setMat4("View", view_);
}

glm::vec3 GraphicsManager::view_dir() {
	return glm::vec3(view_[0][2], view_[1][2], view_[2][2]);
}

void GraphicsManager::prepare_draw_objects() {
	instanced_shader_.use();
	glBindTexture(GL_TEXTURE_2D, atlas_);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	clear_screen(CLEAR_COLOR);
	instanced_shader_.setFloat("texScale", 1.0f / BLOCK_TEXTURE_ATLAS_SIZE);
	instanced_shader_.setMat4("PV", PV_);
	instanced_shader_.setVec3("lightSource", light_source_);
}

void GraphicsManager::prepare_draw_objects_particle_atlas(GLuint atlas) {
	instanced_shader_.use();
	glBindTexture(GL_TEXTURE_2D, atlas);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	instanced_shader_.setFloat("texScale", 1.0f / PARTICLE_TEXTURE_ATLAS_SIZE);
	instanced_shader_.setMat4("PV", PV_);
	instanced_shader_.setVec3("lightSource", light_source_);
}

void GraphicsManager::draw_objects() {
	cube.draw();
	top_cube.draw();
	six_squares.draw();
	windshield.draw();

	diamond.draw();
	top_diamond.draw();
	six_squares_diamond.draw();
	windshield_diamond.draw();

	cube_edges.draw();
	flag.draw();

	square_flat.draw();

	// These can be transparent
	square_0.draw();
	square_1.draw();
	square_2.draw();
	square_3.draw();
}

void GraphicsManager::post_rendering() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_SCISSOR_TEST);
	glViewport(window_->viewport_pos_[0], window_->viewport_pos_[1], window_->viewport_size_[0], window_->viewport_size_[1]);
	post_shader_.use();
	post_shader_.setFloat("shadow", shadow_);
	glBindVertexArray(screen_vao_);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, color_tex_);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

TextRenderer::TextRenderer(FontManager* fonts) :
	fonts_{ fonts }, text_shader_{ fonts->text_shader_ } {
	ui_shader_.use();
	ui_shader_.setFloat("texScale", 1.0f / PARTICLE_TEXTURE_ATLAS_SIZE);
	glGenTextures(1, &ui_atlas_);
	glBindTexture(GL_TEXTURE_2D, ui_atlas_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int width, height, channels;
	unsigned char *texture_data = stbi_load("resources/particles.png", &width, &height, &channels, STBI_rgb_alpha);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	stbi_image_free(texture_data);
}

// These StringDrawers are self-owning, so we have to kill them here
TextRenderer::~TextRenderer() {
	for (auto& p : string_drawers_) {
		if (p.alive_) {
			p.drawer_->cleanup();
		}
	}
}

void TextRenderer::draw() {
	update_drawers();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	draw_ui();
	draw_text();
}

void TextRenderer::update_drawers() {
	std::vector<ProtectedStringDrawer> new_drawers{};
	for (auto& p : string_drawers_) {
		if (p.alive_) {
			if (p.drawer_->active_) {
				p.drawer_->update();
				new_drawers.push_back(std::move(p));
			} else {
				p.drawer_->cleanup();
			}
		}
	}
	string_drawers_ = std::move(new_drawers);
}

void TextRenderer::reset() {
	string_drawers_.clear();
}

void TextRenderer::draw_ui() {
	ui_shader_.use();
	glBindTexture(GL_TEXTURE_2D, ui_atlas_);
	for (auto& p : string_drawers_) {
		p.drawer_->render_bg(&ui_shader_);
	}
}

void TextRenderer::draw_text() {
	text_shader_->use();
	for (auto& p : string_drawers_) {
		p.drawer_->render();
	}
}

void TextRenderer::toggle_string_drawer(StringDrawer* drawer, bool active) {
	if (active) {
		string_drawers_.push_back(ProtectedStringDrawer(drawer));
	} else {
		drawer->kill_instance();
	}
}