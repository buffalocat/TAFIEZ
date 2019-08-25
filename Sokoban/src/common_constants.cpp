#include "stdafx.h"
#include "common_constants.h"

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 900;

const int MESH_SIZE = 50;

const double ORTHO_WIDTH = (double)SCREEN_WIDTH / (double)MESH_SIZE;
const double ORTHO_HEIGHT = (double)SCREEN_HEIGHT / (double)MESH_SIZE;

const double FOV_VERTICAL = glm::radians(60.0);
const double ASPECT_RATIO = (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT;

const double DEFAULT_CAM_RADIUS = 10.0;
const double DEFAULT_CAM_TILT = 0.3;
const double DEFAULT_CAM_ROTATION = 0.0;

const int DEFAULT_BOARD_WIDTH = 17;
const int DEFAULT_BOARD_HEIGHT = 13;
const int DEFAULT_BOARD_DEPTH = 3;

const int HORIZONTAL_MOVEMENT_FRAMES = 8;
const int SWITCH_RESPONSE_FRAMES = 4;
const int COLOR_CHANGE_MOVEMENT_FRAMES = 4;
const int TOGGLE_RIDING_MOVEMENT_FRAMES = 4;
const int FALL_MOVEMENT_FRAMES = 4;
const int MAX_COOLDOWN = 8;

const int UNDO_COMBO_FIRST = 4;
const int UNDO_COMBO_SECOND = 12;

const int UNDO_COOLDOWN_FIRST = 8;
const int UNDO_COOLDOWN_SECOND = 4;
const int UNDO_COOLDOWN_FINAL = 2;

const int MAX_UNDO_DEPTH = 1000;

const int FAST_MAP_MOVE = 10;

const unsigned int GENERIC_WALL_ID = 1;
const unsigned int WORLD_RESET_GLOBAL_ID = 145342541;

// NOTE: the order matters here, for serialization reasons!
const Point3 DIRECTIONS[6] = { {-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0}, {0,0,1}, {0,0,-1} };
const Point3 H_DIRECTIONS[4] = { {-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0} };

const int MAX_ROOM_DIMS = 255;

const std::filesystem::path MAPS_MAIN = std::filesystem::path("maps") / "main";
const std::filesystem::path MAPS_TEMP = std::filesystem::path("maps") / "temp";

const std::string NEW_FILE_START_MAP = "story1";
const std::string WORLD_RESET_START_MAP = "story2";

const std::string Fonts::KALAM_BOLD = "resources/kalam/Kalam-Bold.ttf";
const std::string Fonts::ABEEZEE = "resources/abeezee/ABeeZee-Regular.otf";