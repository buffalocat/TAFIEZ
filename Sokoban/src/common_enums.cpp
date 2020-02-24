#include "stdafx.h"
#include "common_enums.h"
#include "common_constants.h"

Direction point_to_dir(Point3 p) {
	if (p.x) {
		return static_cast<Direction>(p.x + 2);
	} else if (p.y) {
		return static_cast<Direction>(p.y + 3);
	} else if (p.z) {
		return static_cast<Direction>(5 + (1 - p.z) / 2);
	} else {
		return Direction::NONE;
	}
}

Point3 dir_to_point(Direction dir) {
	return DIRECTIONS[static_cast<int>(dir) - 1];
}