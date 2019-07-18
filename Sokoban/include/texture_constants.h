#pragma once

#include <glm/glm.hpp>

// The atlas is a square of 2^k * 2^k square textures
const int TEXTURE_ATLAS_SIZE = 4;

// Reading off the texture atlas starting from top, left to right
enum class BlockTexture {
	LightEdges = 0,
	Corners = 1,
	BrokenEdges = 2,
	Edges = 3,
	Cross = 4,
	SwitchUp = 5,
	SwitchDown = 6,
	Blank = 7,
	AutoBlock = 8,
	Car = 12,
};

BlockTexture operator |(BlockTexture a, BlockTexture b);

glm::vec2 tex_to_vec(BlockTexture tex_id);