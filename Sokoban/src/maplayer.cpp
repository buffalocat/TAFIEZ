#include "stdafx.h"
#include "maplayer.h"

#include "delta.h"
#include "gameobject.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "gameobjectarray.h"


bool MapRect::contains(Point2 p) {
    return (x <= p.x) && (p.x < x + w) && (y <= p.y) && (p.y < y + h);
}


MapLayer::MapLayer(RoomMap* map, int width, int height, int z) :
	parent_map_{ map }, map_{}, width_{ width }, height_{ height }, z_{ z } {
	for (int i = 0; i != width; ++i) {
		map_.push_back(std::vector<int>(height, 0));
	}
}

MapLayer::~MapLayer() {}

int& MapLayer::at(Point2 pos) {
    return map_[pos.x][pos.y];
}

void MapLayer::apply_to_rect(MapRect rect, GameObjIDFunc& f) {
    for (int i = rect.x; i < rect.x + rect.w; ++i) {
        for (int j = rect.y; j < rect.y + rect.h; ++j) {
            if (int id = map_[i][j]) {
                f(id);
            }
        }
    }
}

void MapLayer::apply_to_rect_with_pos(MapRect rect, GameObjIDPosFunc& f) {
	for (int i = rect.x; i < rect.x + rect.w; ++i) {
		for (int j = rect.y; j < rect.y + rect.h; ++j) {
			if (int id = map_[i][j]) {
				f(id, { i,j,z_ });
			}
		}
	}
}

void MapLayer::shift_by(int dx, int dy, int dz) {
    width_ += dx;
    height_ += dy;
	z_ += dz;
    if (dy < 0) {
        for (auto& col : map_) {
			col.erase(col.begin(), col.begin() - dy);
        }
    } else if (dy > 0) {
        for (auto& col : map_) {
			col.insert(col.begin(), dy, 0);
        }
    }
    if (dx < 0) {
        map_.erase(map_.begin(), map_.begin() - dx);
    } else {
        map_.insert(map_.begin(), dx, std::vector<int>(height_,0));
    }
}

void MapLayer::extend_by(int dx, int dy) {
    width_ += dx;
    height_ += dy;
    if (dy < 0) {
        for (auto& col : map_) {
			col.erase(col.end() + dy, col.end());
        }
    } else if (dy > 0) {
        for (auto& col : map_) {
			col.insert(col.end(), dy, 0);
        }
    }
    if (dx < 0) {
        map_.erase(map_.end() + dx, map_.end());
    } else {
        map_.insert(map_.end(), dx, std::vector<int>(height_,0));
    }
}

void MapLayer::serialize_wall_runs(MapFileO& file) {
	for (auto& col : map_) {
		int count = 0;
		bool wall = false;
		for (int id : col) {
			if ((id == GENERIC_WALL_ID) != wall) {
				wall = !wall;
				file << count;
				count = 0;
			}
			++count;
		}
		file << count;
	}
}

void MapLayer::deserialize_wall_runs(MapFileI& file) {
	for (auto& col : map_) {
		int y = 0;
		while (y < height_) {
			y += file.read_byte();
			if (y == height_) {
				break;
			}
			for (int i = file.read_byte(); i > 0; --i) {
				col[y] = GENERIC_WALL_ID;
				++y;
			}
		}
	}
}