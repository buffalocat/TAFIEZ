#ifndef COLOR_CONSTANTS_H_INCLUDED
#define COLOR_CONSTANTS_H_INCLUDED

#include <glm/glm.hpp>

// TODO: make the color system smarter!
enum {
	NO_COLOR = 0,
	GREEN = 1,
	LIGHT_PINK = 2,
	PURPLE = 3,
	DARK_PURPLE = 4,
	BLUE = 5,
	SALMON = 6,
	RED = 7,
	BLACK = 8,
	LIGHT_GREY = 9,
	GOLD = 10,
	YELLOW = 11,
	GREY = 12,
	WHITE = 13,
	DARK_GREY = 14,
	DARK_BLUE = 15,
	BRIGHT_PURPLE = 16,
	ORANGE = 17,
	LIGHT_ORANGE = 18,
	LIGHT_BLUE = 19,
	LIGHT_GREEN = 20,
	DARK_SALMON = 21,
	PINK = 22,
	NUM_COLORS,
};

extern const glm::vec4 COLOR_VECTORS[NUM_COLORS];


const int NUM_GREYS = 8;

const double GREY_INTENSITIES[NUM_GREYS] = { 0.1f, 0.17f, 0.25f, 0.37f, 0.5f, 0.64f, 0.79f, 0.95f};

#define MAKE_GREY(i) glm::vec4{i, i, i, 1.0f},

const glm::vec4 GREY_VECTORS[NUM_GREYS] {
	MAKE_GREY(0.1f)
	MAKE_GREY(0.17f)
	MAKE_GREY(0.25f)
	MAKE_GREY(0.37f)
	MAKE_GREY(0.5f)
	MAKE_GREY(0.64f)
	MAKE_GREY(0.79f)
	MAKE_GREY(0.95f)
};

#undef MAKE_GREY

#endif // COLOR_CONSTANTS_H_INCLUDED
