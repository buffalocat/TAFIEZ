#include "stdafx.h"
#include "common_constants.h"
#include <glm/glm.hpp>

const double DEFAULT_CAM_RADIUS = 12.0;
const double DEFAULT_CAM_TILT = 0.2;
const double DEFAULT_CAM_ROTATION = 0.0;

const double FOV_VERTICAL = glm::radians(60.0);

const double ASPECT_RATIO = (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT;

const std::filesystem::path MAPS_MAIN = std::filesystem::path("maps") / "main";
const std::filesystem::path MAPS_TEMP = std::filesystem::path("maps") / "temp";