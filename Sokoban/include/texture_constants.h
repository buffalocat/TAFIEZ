#pragma once

#include <glm/glm.hpp>

// The atlas is a square of 2^k * 2^k square textures
const int TEXTURE_ATLAS_SIZE = 8;

// Reading off the texture atlas starting from top, left to right
enum class BlockTexture {
	LightEdges = 0,
	Corners = 1,
	BrokenEdges = 2,
	Edges = 3,
	Cross = 16,
	SwitchUp = 17,
	SwitchDown = 18,
	Blank = 19,
	Wall = 20,
	Door = 21,
	AccentSquare = 22,

	// "Additive" textures
	Default = 0,
	Car = 4,
	AutoBlock = 8,
	PuppetBlock = 12,
};

BlockTexture operator |(BlockTexture a, BlockTexture b);

glm::vec2 tex_to_vec(BlockTexture tex_id);