#pragma once

#include "model.h"
#include <glm/glm.hpp>

class Shader;
struct InstanceData;
enum class BlockTexture;

class ModelInstancer {
public:
	ModelInstancer(std::string const& path);
	virtual ~ModelInstancer();
	void push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, int color);
	void push_instance(glm::vec3 pos, glm::vec3 scale, BlockTexture tex_id, glm::vec4 color);
	virtual void draw(Shader shader) = 0;
	void set_instance_attributes(int VAO);
protected:
	Model model_;
	std::vector<InstanceData> instances_;
	unsigned int buffer_;
	void setup_buffer();
	void fill_buffer();
};

class DynamicInstancer : public ModelInstancer {
public:
	DynamicInstancer(std::string const& path);
	~DynamicInstancer();
	void draw(Shader shader);
};

class WallInstancer : public ModelInstancer {
public:
	WallInstancer(std::string const& path);
	~WallInstancer();
	void draw(Shader shader);
};