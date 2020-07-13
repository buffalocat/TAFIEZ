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
	SolidSquare = 0,
	BlurDisk = 2,
	Diamond = 4,

	//Textbox
	TextboxTL = 17,
	TextboxTR = 18,
	TextboxT = 20,
	TextboxR = 22,
	TextboxBL = 25,
	TextboxBR = 26,
	TextboxL = 36,
	TextboxB = 38,

	//Other
	Camera = 40,
	VertLine = 41,
	HorLine = 48,
	TeeLine = 49,
	HorDashes = 56,
	VertDashes = 57,
};

glm::vec2 tex_to_vec(ParticleTexture tex_id);