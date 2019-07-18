#pragma once

#include "model.h"
#include <glm/glm.hpp>

class Shader;
struct InstanceData;
enum class BlockTexture;

class ModelInstancer {
public:
	ModelInstancer(std::string const& path);
	~ModelInstancer();
	void push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, int color);
	void draw(Shader shader);
private:
	Model model_;
	std::vector<InstanceData> instances_;
	unsigned int buffer_;
	void setup_buffer();
	void fill_buffer();
};