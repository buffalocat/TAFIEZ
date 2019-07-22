#ifndef WALL_H
#define WALL_H

#include "gameobject.h"

class Wall: public GameObject {
public:
    Wall();
	Wall(Point3);
    ~Wall();

	static std::unique_ptr<GameObject> deserialize(MapFileI& file);
    std::string name();
	ObjCode obj_code();

    bool skip_serialization();
	void draw(GraphicsManager*);
    static void draw(GraphicsManager* gfx, Point3 p);
};

#endif // WALL_H
