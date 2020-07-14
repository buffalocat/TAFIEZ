#include "stdafx.h"
#include "common_constants.h"

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 900;

const int MESH_SIZE = 50;

const double ORTHO_WIDTH = (double)SCREEN_WIDTH / (double)MESH_SIZE;
const double ORTHO_HEIGHT = (double)SCREEN_HEIGHT / (double)MESH_SIZE;

const double FOV_VERTICAL = glm::radians(60.0);
const double ASPECT_RATIO = (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT;

const double TWO_PI = 6.28318530718;

const double DEFAULT_CAM_RADIUS = 10.0;
const double DEFAULT_CAM_TILT = 0.3;
const double DEFAULT_CAM_ROTATION = 0.0;

const double MAX_CAM_TILT = 1.5;
const double MIN_CAM_TILT = 0.0;

const double FREE_CAM_TILT_SPEED = 0.01;
const double FREE_CAM_ROT_SPEED = 0.02;

const int DEFAULT_BOARD_WIDTH = 17;
const int DEFAULT_BOARD_HEIGHT = 13;
const int DEFAULT_BOARD_DEPTH = 3;

const int HALF_STEP_FRAMES = 5;
const int FULL_STEP_FRAMES = 2 * HALF_STEP_FRAMES;

const int HORIZONTAL_MOVEMENT_FRAMES = FULL_STEP_FRAMES;
const int JUMP_MOVEMENT_FRAMES = FULL_STEP_FRAMES;
const int SWITCH_RESPONSE_FRAMES = HALF_STEP_FRAMES;
const int FALL_MOVEMENT_FRAMES = HALF_STEP_FRAMES;

const int COLOR_CHANGE_MOVEMENT_FRAMES = FULL_STEP_FRAMES;
const int TOGGLE_RIDING_MOVEMENT_FRAMES = FULL_STEP_FRAMES;
const int MAX_COOLDOWN = FULL_STEP_FRAMES;

// Note: MAX_COOLDOWN <= any individual move step's frames

const int UNDO_COMBO_FIRST = 4;
const int UNDO_COMBO_SECOND = 12;

const int UNDO_COOLDOWN_FIRST = FULL_STEP_FRAMES;
const int UNDO_COOLDOWN_SECOND = HALF_STEP_FRAMES;
const int UNDO_COOLDOWN_FINAL = 2;

const int MAX_UNDO_DEPTH = 1 << 16;

const int FAST_MAP_MOVE = 10;

const unsigned int GENERIC_WALL_ID = 1;

// NOTE: the order matters here, for serialization reasons!
const Point3 DIRECTIONS[6] = { {-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0}, {0,0,1}, {0,0,-1} };
const Point3 H_DIRECTIONS[4] = { {-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0} };

const int MAX_ROOM_DIMS = 255;

const std::filesystem::path MAPS_MAIN = std::filesystem::path("maps") / "main";
const std::filesystem::path MAPS_TEMP = std::filesystem::path("maps") / "temp";

const std::string NEW_FILE_START_MAP = "T";
const std::string WORLD_RESET_START_MAP = "HA";

const std::string Fonts::KALAM_BOLD = "resources/kalam/Kalam-Bold.ttf";
const std::string Fonts::ABEEZEE = "resources/abeezee/ABeeZee-Regular.otf";