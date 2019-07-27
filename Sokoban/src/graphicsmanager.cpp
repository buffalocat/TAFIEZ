#include "stdafx.h"
#include "graphicsmanager.h"
#include "texture_constants.h"

#include "model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GraphicsManager::GraphicsManager(GLFWwindow* window):
	window_{ window },
	shader_{ Shader("shaders/shader.vs", "shaders/shader.fs") },
	instanced_shader_{ Shader("shaders/instanced_shader.vs", "shaders/instanced_shader.fs") },
	cube{ DynamicInstancer("resources/uniform_cube.obj") },
	top_cube{ DynamicInstancer("resources/top_cube.obj") },
	diamond{ DynamicInstancer("resources/diamond.obj") },
	six_squares{ DynamicInstancer("resources/uniform_cube.obj") },
	wall{ WallInstancer("resources/uniform_cube.obj") }
{
	instanced_shader_.use();
	load_texture_atlas();
	instanced_shader_.setFloat("texScale", 1.0f/ TEXTURE_ATLAS_SIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLFWwindow* GraphicsManager::window() {
    return window_;
}

void GraphicsManager::load_texture_atlas() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, channels;
    unsigned char *texture_data = stbi_load("resources/textures.png", &width, &height, &channels, STBI_rgb_alpha);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
    stbi_image_free(texture_data);
}

void GraphicsManager::set_PV(glm::mat4 projection, glm::mat4 view) {
	glm::mat4 PV = projection * view;
    if (PV != PV_) {
        PV_ = PV;
		instanced_shader_.setMat4("PV", PV);
    }
}

void GraphicsManager::draw() {
	cube.draw(instanced_shader_);
	top_cube.draw(instanced_shader_);
	diamond.draw(instanced_shader_);
	six_squares.draw(instanced_shader_);
	wall.draw(instanced_shader_);
}