#include "stdafx.h"
#include "color_constants.h"
#include "globalflagconstants.h"

const glm::vec4 COLOR_VECTORS[] = {
	glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
	glm::vec4{0.2f, 0.6f, 0.2f, 1.0f},
	glm::vec4{0.9f, 0.6f, 0.7f, 1.0f},
	glm::vec4{0.7f, 0.5f, 0.9f, 1.0f},
	glm::vec4{0.3f, 0.2f, 0.5f, 1.0f},
	glm::vec4{0.0f, 0.1f, 0.8f, 1.0f},
	glm::vec4{1.0f, 0.4f, 0.4f, 1.0f},
	glm::vec4{0.6f, 0.0f, 0.1f, 1.0f},
	glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
	glm::vec4{0.7f, 0.7f, 0.7f, 1.0f},
	glm::vec4{1.0f, 0.7f, 0.3f, 1.0f},
	glm::vec4{0.7f, 0.7f, 0.3f, 1.0f},
	glm::vec4{0.2f, 0.2f, 0.2f, 1.0f},
	glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
	glm::vec4{0.1f, 0.1f, 0.1f, 1.0f},
	glm::vec4{0.1f, 0.2f, 0.4f, 1.0f},
	glm::vec4{0.3f, 0.1f, 0.5f, 1.0f},
	glm::vec4{1.0f, 0.4f, 0.0f, 1.0f},
	glm::vec4{1.0f, 0.7f, 0.5f, 1.0f},
	glm::vec4{0.6f, 0.6f, 0.8f, 1.0f},
	glm::vec4{0.6f, 0.8f, 0.6f, 1.0f},
	glm::vec4{0.5f, 0.1f, 0.2f, 1.0f},
	glm::vec4{1.0f, 0.3f, 1.0f, 1.0f},
};

const glm::vec4 ZONE_CLEAR_COLORS[] = {
	glm::vec4{0.89f, 0.86f, 0.48f, 1.0f},
	glm::vec4{0.97f, 0.47f, 0.61f, 1.0f},
	glm::vec4{0.97f, 0.81f, 0.44f, 1.0f},
	glm::vec4{0.78f, 0.44f, 0.95f, 1.0f},
	glm::vec4{0.93f, 0.69f, 0.33f, 1.0f},
	glm::vec4{0.89f, 0.46f, 0.87f, 1.0f},
	glm::vec4{0.53f, 0.8f, 0.23f, 1.0f},
	glm::vec4{0.38f, 0.62f, 0.75f, 1.0f},
	glm::vec4{0.89f, 0.87f, 0.57f, 1.0f},
	glm::vec4{0.62f, 0.96f, 0.5f, 1.0f},
	glm::vec4{0.76f, 0.97f, 0.44f, 1.0f},
	glm::vec4{0.49f, 0.96f, 0.7f, 1.0f},
	glm::vec4{0.4f, 0.73f, 0.9f, 1.0f},
	glm::vec4{0.44f, 0.95f, 0.77f, 1.0f},
	glm::vec4{0.95f, 0.64f, 0.44f, 1.0f},
	glm::vec4{0.67f, 0.51f, 0.87f, 1.0f},
	glm::vec4{1.0f, 0.53f, 0.53f, 1.0f},
	glm::vec4{0.95f, 0.63f, 0.73f, 1.0f},
	glm::vec4{0.52f, 0.57f, 0.98f, 1.0f},
	glm::vec4{0.58f, 0.96f, 0.78f, 1.0f},
	glm::vec4{1.0f, 0.65f, 0.65f, 1.0f},
	glm::vec4{0.97f, 0.44f, 0.44f, 1.0f},
	glm::vec4{0.93f, 0.66f, 0.47f, 1.0f},
	glm::vec4{0.82f, 0.36f, 0.77f, 1.0f},
	glm::vec4{0.99f, 0.57f, 0.78f, 1.0f},
	glm::vec4{0.32f, 0.65f, 0.25f, 1.0f},
	glm::vec4{0.3f, 0.64f, 0.82f, 1.0f},
	glm::vec4{0.42f, 0.86f, 0.95f, 1.0f},
	glm::vec4{0.84f, 0.6f, 0.22f, 1.0f},
	glm::vec4{0.82f, 0.94f, 0.45f, 1.0f},
	glm::vec4{0.51f, 0.47f, 0.94f, 1.0f},
	glm::vec4{0.68f, 0.95f, 0.55f, 1.0f},
	glm::vec4{0.5f, 0.84f, 0.88f, 1.0f},
	glm::vec4{0.39f, 0.71f, 0.4f, 1.0f},
	glm::vec4{0.35f, 0.44f, 0.79f, 1.0f},
	glm::vec4{0.96f, 0.36f, 0.36f, 1.0f},
	glm::vec4{0.57f, 0.65f, 0.87f, 1.0f},
};

glm::vec4 get_zone_clear_color(char zone) {
	return ZONE_CLEAR_COLORS[get_clear_flag_index(zone)];
}

const glm::vec4 CLEAR_COLOR = glm::vec4{ 0.33f, 0.67f, 0.89f, 1.0f };
