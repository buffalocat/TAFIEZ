#include "stdafx.h"
#include "modelinstancer.h"
#include "color_constants.h"
#include "texture_constants.h"
#include "vertexlayout.h"
#include "shader.h"

const int MAX_INSTANCES = 10000;

ModelInstancer::ModelInstancer(std::string const& path) :
	model_{ Model(path) },
	instances_{}
{
	setup_buffer();
}

ModelInstancer::~ModelInstancer() {}

void ModelInstancer::setup_buffer() {
	glGenBuffers(1, &buffer_);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
	glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * sizeof(InstanceData), nullptr, GL_STATIC_DRAW);

	model_.bind_instance_buffer(buffer_);
}

void ModelInstancer::fill_buffer() {
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
	glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * sizeof(InstanceData), nullptr, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, instances_.size() * sizeof(InstanceData), &instances_[0], GL_STATIC_DRAW);
}

void ModelInstancer::push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, int color) {
	instances_.push_back(InstanceData{ pos, scale, tex_to_vec(tex_id), COLORS[color] });
}

void ModelInstancer::draw(Shader shader) {
	fill_buffer();
	model_.draw(shader, instances_.size());
	instances_.clear();
}