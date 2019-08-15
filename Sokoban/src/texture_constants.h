#pragma once

#include <glm/glm.hpp>

// The atlas is a square of 2^k * 2^k square textures
const int TEXTURE_ATLAS_SIZE = 8;

// Reading off the texture atlas starting from top, left to right
enum class BlockTexture {
	// ColoredBlocks
	LightEdges = 0,
	Corners = 1,
	BrokenEdges = 2,
	Edges = 3,
	// Special
	Blank = 16,
	Wall = 17,
	Darker = 18,
	// Double-size
	Door = 32,
	Cross = 33,
	SwitchUp = 34,
	SwitchDown = 35,
	GateBase = 36,
	GateBody = 37,
	GateBasePersistent = 38,
	GateBodyPersistent = 39,
	Sign = 48,
	// Next double-size texture = 49,
	PermSwitchUp = 50,
	PermSwitchDown = 51,

	// "Additive" textures
	Default = 0,
	Car = 4,
	AutoBlock = 8,
	PuppetBlock = 12,
};

BlockTexture operator |(BlockTexture a, BlockTexture b);

glm::vec2 tex_to_vec(BlockTexture tex_id);