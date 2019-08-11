#include "stdafx.h"
#include "graphicsmanager.h"
#include "common_constants.h"
#include "texture_constants.h"
#include "textrenderer.h"
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GraphicsManager::GraphicsManager(GLFWwindow* window) :
	text_{ std::make_unique<TextRenderer>() },
	window_{ window },
	instanced_shader_{ Shader("shaders/instanced_shader.vs", "shaders/instanced_shader.fs") },
	cube{ DynamicInstancer("resources/uniform_cube.obj") },
	top_cube{ DynamicInstancer("resources/top_cube.obj") },
	diamond{ DynamicInstancer("resources/diamond.obj") },
	six_squares{ DynamicInstancer("resources/six_squares.obj") },
	windshield{ SingleDrawer("resources/windshield.obj") },
	windshield_diamond{ SingleDrawer("resources/windshield_diamond.obj") },
	flag{ SingleDrawer("resources/flag.obj") },
	wall{ WallInstancer("resources/uniform_cube.obj") }
{
	text_->init();
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

void GraphicsManager::draw_world() {
	cube.draw();
	top_cube.draw();
	diamond.draw();
	six_squares.draw();
	wall.draw();
}

void GraphicsManager::draw_text() {
	text_->render_text();
}