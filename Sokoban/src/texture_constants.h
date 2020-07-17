#pragma once

// The atlas is a square of 2^k * 2^k square textures
const int BLOCK_TEXTURE_ATLAS_SIZE = 16;

// Reading off the texture atlas starting from top, left to right
enum class BlockTexture {
	// ColoredBlocks
	LightEdges = 0,
	Corners = 1,
	BrokenEdges = 2,
	Edges = 3,

	// "Additive" textures
	Default = 0,
	LockedCar = 4,
	NormalCar = 8,
	ConvertibleCar = 12,
	AutoBlock = 16,
	PuppetBlock = 20,
	HoverCar = 24,
	BindingCar = 28,

	// Double-size
	Sign = 48,
	Door,
	Cross,
	SwitchUp,
	SwitchDown,
	PermSwitchUp,
	PermSwitchDown,
	GateBase,
	GateBody,
	GateBasePersistent,
	GateBodyPersistent,
	GateBodyCorrupt,
	Incinerator,

	FlagSwitch0 = 80,
	FlagSwitch1,
	FlagSwitch2,
	FlagSwitch3,

	// Padded Single-Size
	FlagSigil = 129,
	Darker = 131,
	Wall = 133,
	Blank = 135,
};

BlockTexture operator |(BlockTexture a, BlockTexture b);

glm::vec2 tex_to_vec(BlockTexture tex_id);


// The atlas is a square of 2^k * 2^k square textures
const int PARTICLE_TEXTURE_ATLAS_SIZE = 8;

// Reading off the texture atlas starting from top, left to right
enum class ParticleTexture {
	// Particles
	SolidSquare = 0,
	BlurDisk = 2,
	Diamond = 4,

	// Textbox
	TextboxTL = 17,
	TextboxTR = 18,
	TextboxT = 20,
	TextboxR = 22,
	TextboxBL = 25,
	TextboxBR = 26,
	TextboxL = 36,
	TextboxB = 38,

	// Misc
	Camera = 40,

	// Map Symbols
	VertLine = 41,
	HorLine = 48,
	TeeLine = 49,
	HorDashes = 56,
	VertDashes = 57,

	GreySolid = 51,
	GreyDashed = 52,
	GoldSolid = 53,
	GoldDashed = 54,
	PinkSolid = 55,

	Alpha = 59,
	Beta = 60,
	Gamma = 61,
	Delta = 62,
	Omega = 63,
};

glm::vec2 tex_to_vec(ParticleTexture tex_id);