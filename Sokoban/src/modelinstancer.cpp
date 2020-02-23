#include "stdafx.h"
#include "modelinstancer.h"
#include "color_constants.h"
#include "texture_constants.h"
#include "vertexlayout.h"
#include "shader.h"

ModelInstancer::ModelInstancer(std::string path) :
	model_{ Model(path) },
	instances_{}
{
	setup_buffer();
}

ModelInstancer::~ModelInstancer() {}

void ModelInstancer::setup_buffer() {
	glGenBuffers(1, &buffer_);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);

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
}

void ModelInstancer::fill_buffer() {
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
	glBufferData(GL_ARRAY_BUFFER, instances_.size() * sizeof(InstanceData), instances_.data(), GL_STATIC_DRAW);
}

void ModelInstancer::push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, int color) {
	instances_.push_back(InstanceData{ pos, scale, tex_to_vec(tex_id), COLOR_VECTORS[color] });
}

void ModelInstancer::push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, glm::vec4 color) {
	instances_.push_back(InstanceData{ pos, scale, tex_to_vec(tex_id), color });
}

DynamicInstancer::DynamicInstancer(std::string path) : ModelInstancer(path) {}

DynamicInstancer::~DynamicInstancer() {}

void DynamicInstancer::draw() {
	fill_buffer();
	model_.draw_instanced((unsigned int)instances_.size());
	instances_.clear();
}

StaticInstancer::StaticInstancer(std::string path) : ModelInstancer(path) {}

StaticInstancer::~StaticInstancer() {}

void StaticInstancer::draw() {
	model_.draw_instanced((unsigned int)instances_.size());
}
