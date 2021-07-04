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
	Sign = 64,
	Door,
	Cross,
	SwitchUp,
	SwitchDown,
	PermSwitchUp,
	PermSwitchDown,
	GateBase,
	GateBody,
	GateBasePersistentOff,
	GateBodyPersistentOn,
	GateBasePersistentOn,
	GateBodyPersistentOff,
	GateBodyPersistentDisconnected,
	GateBodyCorrupt,
	GateBaseExtended,

	// Double-size Row 2
	Incinerator = 96,
	IncineratorPersistentActiveOff,
	IncineratorPersistentActiveOn,
	IncineratorPersistentInactiveOn,
	IncineratorPersistentInactiveOff,
	FlagSwitch0,
	FlagSwitch1,
	FlagSwitch2,
	FlagSwitch3,
	AutosaveFresh,
	AutosaveUsed,

	// Padded Single-Size
	FlagSigil = 145,
	Darker = 147,
	Wall = 149,
	Blank = 151,
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
	Camera = 48,

	// Map Symbols
	VertLine = 49,
	HorLine = 40,
	TeeLine = 41,
	HorDashes = 56,
	VertDashes = 57,

	SolidBox = 51,
	DashedBox = 52,

	Alpha = 59,
	Beta = 60,
	Gamma = 61,
	Delta = 62,
	Omega = 63,
};

glm::vec2 tex_to_vec(ParticleTexture tex_id);