#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include "point.h"

// True constants
const int MAX_COLOR_CYCLE = 5; // Must be defined here because it's an array length

extern const double DEFAULT_CAM_RADIUS;
extern const double DEFAULT_CAM_TILT;
extern const double DEFAULT_CAM_ROTATION;

extern const int GLOBAL_WALL_ID;

// NOTE: the order matters here, for serialization reasons!
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

// Must satisfy MAX_COOLDOWN < HORIZONTAL_MOVEMENT_FRAMES for smooth motion
extern const int HORIZONTAL_MOVEMENT_FRAMES;
extern const int SWITCH_RESPONSE_FRAMES;
extern const int COLOR_CHANGE_MOVEMENT_FRAMES;
extern const int FALL_MOVEMENT_FRAMES;
extern const int MAX_COOLDOWN;

extern const int AREA_NAME_DISPLAY_FRAMES;
extern const int AREA_NAME_FADE_FRAMES;

extern const int UNDO_COMBO_FIRST;
extern const int UNDO_COMBO_SECOND;
extern const int UNDO_COOLDOWN_FIRST;
extern const int UNDO_COOLDOWN_SECOND;
extern const int UNDO_COOLDOWN_FINAL;

extern const int MAX_UNDO_DEPTH;

extern const int FAST_MAP_MOVE;

extern const std::filesystem::path MAPS_MAIN;
extern const std::filesystem::path MAPS_TEMP;

#endif // COMMON_CONSTANTS_H
