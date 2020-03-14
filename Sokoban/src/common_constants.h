#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include "point.h"

// Array lengths
const int MAX_COLOR_CYCLE = 5;
const unsigned int MAX_LONG_STRING_SIZE = 4096;

extern const unsigned int GENERIC_WALL_ID;

extern const Point3 DIRECTIONS[6];
extern const Point3 H_DIRECTIONS[4];

extern const int MAX_ROOM_DIMS;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

extern const int MESH_SIZE;

extern const double FOV_VERTICAL;
extern const double ASPECT_RATIO;

extern const double ORTHO_WIDTH;
extern const double ORTHO_HEIGHT;

extern const int DEFAULT_BOARD_WIDTH;
extern const int DEFAULT_BOARD_HEIGHT;
extern const int DEFAULT_BOARD_DEPTH;

extern const int FAST_MAP_MOVE;

// Must satisfy MAX_COOLDOWN < HORIZONTAL_MOVEMENT_FRAMES for smooth motion
extern const int HORIZONTAL_MOVEMENT_FRAMES;
extern const int SWITCH_RESPONSE_FRAMES;
extern const int COLOR_CHANGE_MOVEMENT_FRAMES;
extern const int TOGGLE_RIDING_MOVEMENT_FRAMES;
extern const int JUMP_MOVEMENT_FRAMES;
extern const int FALL_MOVEMENT_FRAMES;
extern const int MAX_COOLDOWN;

extern const int UNDO_COMBO_FIRST;
extern const int UNDO_COMBO_SECOND;
extern const int UNDO_COOLDOWN_FIRST;
extern const int UNDO_COOLDOWN_SECOND;
extern const int UNDO_COOLDOWN_FINAL;

extern const int MAX_UNDO_DEPTH;

extern const double TWO_PI;

extern const double DEFAULT_CAM_RADIUS;
extern const double DEFAULT_CAM_TILT;
extern const double DEFAULT_CAM_ROTATION;

extern const double MAX_CAM_TILT;
extern const double MIN_CAM_TILT;

extern const double FREE_CAM_TILT_SPEED;
extern const double FREE_CAM_ROT_SPEED;

extern const std::filesystem::path MAPS_MAIN;
extern const std::filesystem::path MAPS_TEMP;

extern const std::string NEW_FILE_START_MAP;
extern const std::string WORLD_RESET_START_MAP;

namespace Fonts {
	extern const std::string KALAM_BOLD;
	extern const std::string ABEEZEE;
}

#endif // COMMON_CONSTANTS_H
