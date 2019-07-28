#ifndef PLAYER_H
#define PLAYER_H

#include "gameobject.h"

class Car;

class Player: public GameObject {
public:
    Player(Point3 pos, RidingState state);
    virtual ~Player();

	void serialize(MapFileO& file);
	static std::unique_ptr<GameObject> deserialize(MapFileI& file);

    std::string name();
    ObjCode obj_code();
	int color();

	FPoint3 cam_pos();

    void toggle_riding(RoomMap* room_map, DeltaFrame*);
    Car* get_car(RoomMap* room_map, bool strict);

    virtual void collect_special_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    void draw(GraphicsManager*);

    RidingState state_;
};

#endif // PLAYER_H
