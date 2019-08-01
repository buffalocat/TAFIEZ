#ifndef MAPLAYER_H
#define MAPLAYER_H

#include "common_enums.h"
#include "point.h"

class GameObject;
class DeltaFrame;
class RoomMap;
class MapFileI;
class MapFileO;
class GraphicsManager;
class GameObjectArray;

struct MapRect {
    int x;
    int y;
    int w;
    int h;
    bool contains(Point2);
};

using GameObjIDFunc = std::function<void(int)>;
using GameObjIDPosFunc = std::function<void(int, Point3)>;

class MapLayer {
public:
	MapLayer(RoomMap*, int width, int height, int z);
	~MapLayer();

	int& at(Point2 pos);

	void apply_to_rect(MapRect, GameObjIDFunc&);
	void apply_to_rect_with_pos(MapRect, GameObjIDPosFunc&);
	void extend_by(int dx, int dy);
	void shift_by(int dx, int dy, int dz);

	void serialize_wall_runs(MapFileO& file);
	void deserialize_wall_runs(MapFileI& file);

protected:
    RoomMap* parent_map_;
	std::vector<std::vector<int>> map_;
	int width_;
	int height_;
	int z_;
};


#endif // MAPLAYER_H
