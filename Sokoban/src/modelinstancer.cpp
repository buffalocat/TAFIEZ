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
	// TODO: let MAX_INSTANCES depend on model explicitly
	glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * sizeof(InstanceData), nullptr, GL_STATIC_DRAW);

	for (int VAO : model_.gather_mesh_vao()) {
		set_instance_attributes(VAO);
	}
}

void ModelInstancer::set_instance_attributes(int VAO) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
	// pos offset
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, PosOffset));
	glVertexAttribDivisor(3, 1);
	// scales
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, Scale));
	glVertexAttribDivisor(4, 1);
	// tex offset
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, TexOffset));
	glVertexAttribDivisor(5, 1);
	// color
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, Color));
	glVertexAttribDivisor(6, 1);
	glBindVertexArray(0);
}

void ModelInstancer::fill_buffer() {
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
	glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * sizeof(InstanceData), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, instances_.size() * sizeof(InstanceData), instances_.data());
}

void ModelInstancer::push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, int color) {
	instances_.push_back(InstanceData{ pos, scale, tex_to_vec(tex_id), COLOR_VECTORS[color] });
}

void ModelInstancer::push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, glm::vec4 color) {
	instances_.push_back(InstanceData{ pos, scale, tex_to_vec(tex_id), color });
}

DynamicInstancer::DynamicInstancer(std::string const& path) : ModelInstancer(path) {}

DynamicInstancer::~DynamicInstancer() {}

void DynamicInstancer::draw(Shader shader) {
	fill_buffer();
	model_.draw(shader, (unsigned int)instances_.size());
	instances_.clear();
}

WallInstancer::WallInstancer(std::string const& path) : ModelInstancer(path) {}

WallInstancer::~WallInstancer() {}

void WallInstancer::draw(Shader shader) {
	model_.draw(shader, (unsigned int)instances_.size());
}
