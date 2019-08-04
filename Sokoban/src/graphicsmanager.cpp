#include "stdafx.h"
#include "graphicsmanager.h"
#include "common_constants.h"
#include "texture_constants.h"
#include "textrenderer.h"
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GraphicsManager::GraphicsManager(GLFWwindow* window) :
	window_{ window },
	text_{ std::make_unique<TextRenderer>() },
	instanced_shader_{ Shader("shaders/instanced_shader.vs", "shaders/instanced_shader.fs") },
	cube{ DynamicInstancer("resources/uniform_cube.obj") },
	top_cube{ DynamicInstancer("resources/top_cube.obj") },
	diamond{ DynamicInstancer("resources/diamond.obj") },
	six_squares{ DynamicInstancer("resources/six_squares.obj") },
	wall{ WallInstancer("resources/uniform_cube.obj") }
{
	text_->init();
	instanced_shader_.use();
	load_texture_atlas();
	instanced_shader_.setFloat("texScale", 1.0f / TEXTURE_ATLAS_SIZE);
	glEnable(GL_DEPTH_TEST);
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

void GraphicsManager::set_PV(glm::mat4 PV) {
	instanced_shader_.use();
	if (PV != PV_) {
		PV_ = PV;
		instanced_shader_.setMat4("PV", PV);
	}
}

void GraphicsManager::render_text(std::string text, float opacity) {
	static const float sx = 2.0f / SCREEN_WIDTH;
	static const float sy = 2.0f / SCREEN_HEIGHT;
	text_->render(text.c_str(), 0, 0.8f, sx, sy, opacity);
}

void GraphicsManager::draw_world() {
	glBindTexture(GL_TEXTURE_2D, atlas_);
	cube.draw();
	top_cube.draw();
	diamond.draw();
	six_squares.draw();
	wall.draw();
}