#ifndef WALL_H
#define WALL_H

#include "gameobject.h"

class Wall: public Block {
public:
    Wall();
	Wall(Point3);
	virtual ~Wall();

	static std::unique_ptr<GameObject> deserialize(MapFileI& file, Point3 pos);
	virtual std::string name();
	virtual ObjCode obj_code();

	std::unique_ptr<GameObject> duplicate(RoomMap*, DeltaFrame*);

	virtual void draw(GraphicsManager*);
    static void draw(GraphicsManager* gfx, Point3 p);
};

class ArtWall : public Wall {
public:
	ArtWall(Point3 pos, int flavor);
	~ArtWall();

	void serialize(MapFileO& file);
	static std::unique_ptr<GameObject> deserialize(MapFileI& file, Point3 pos);
	std::string name();
	ObjCode obj_code();

	void draw(GraphicsManager*);

	int flavor_;
};


#endif // WALL_H
