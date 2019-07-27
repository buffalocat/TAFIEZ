#ifndef COLOR_CONSTANTS_H_INCLUDED
#define COLOR_CONSTANTS_H_INCLUDED

#include <glm/glm.hpp>

// TODO: make the color system smarter!
enum {
	NO_COLOR = 0,
    GREEN = 1,
    PINK = 2,
    PURPLE = 3,
    DARK_PURPLE = 4,
    BLUE = 5,
    RED = 6,
    DARK_RED = 7,
    BLACK = 8,
    LIGHT_GREY = 9,
    ORANGE = 10,
    YELLOW = 11,
    GREY = 12,
    DARK_GREY = 13,
    WHITE = 14,
};

const glm::vec4 COLOR_VECTORS[] = {
	glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
    glm::vec4{0.3f, 0.9f, 0.4f, 1.0f},
    glm::vec4{0.9f, 0.6f, 0.7f, 1.0f},
    glm::vec4{0.7f, 0.5f, 0.9f, 1.0f},
    glm::vec4{0.3f, 0.2f, 0.6f, 1.0f},
    glm::vec4{0.1f, 0.1f, 0.9f, 1.0f},
    glm::vec4{1.0f, 0.5f, 0.5f, 1.0f},
    glm::vec4{0.6f, 0.0f, 0.1f, 1.0f},
    glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
    glm::vec4{0.7f, 0.7f, 0.7f, 1.0f},
    glm::vec4{1.0f, 0.7f, 0.3f, 1.0f},
    glm::vec4{0.7f, 0.7f, 0.3f, 1.0f},
    glm::vec4{0.2f, 0.2f, 0.2f, 1.0f},
    glm::vec4{0.1f, 0.1f, 0.1f, 1.0f},
    glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
};

const int NUM_GREYS = 8;

const float GREY_INTENSITIES[NUM_GREYS] = { 0.1f, 0.17f, 0.25f, 0.37f, 0.5f, 0.64f, 0.79f, 0.95f};

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
