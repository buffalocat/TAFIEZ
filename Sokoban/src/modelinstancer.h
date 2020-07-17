#pragma once

#include "model.h"

struct InstanceData;
enum class BlockTexture;
enum class ParticleTexture;

class ModelInstancer {
public:
	ModelInstancer(std::string path);
	~ModelInstancer();
	void push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, int color);
	void push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, glm::vec4 color);
	void push_instance(glm::vec3 pos, glm::vec3 scale, ParticleTexture tex_id, int color);
	void push_instance(glm::vec3 pos, glm::vec3 scale, ParticleTexture tex_id, glm::vec4 color);
	void draw();
	void set_instance_attributes(int VAO);
protected:
	Model model_;
	std::vector<InstanceData> instances_;
	unsigned int buffer_;
	void setup_buffer();
	void fill_buffer();
};