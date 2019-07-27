#pragma once

#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct InstanceData {
	glm::vec3 PosOffset;
	glm::vec3 Scale;
	glm::vec2 TexOffset;
	glm::vec4 Color;
};