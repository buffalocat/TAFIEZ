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

	void set_free();
	void set_strictest(RoomMap* room_map);
	void validate_state(RoomMap* room_map);
	void toggle_riding(RoomMap* room_map, DeltaFrame*);

	bool bound();
    Car* car_riding();
	Car* car_bound(RoomMap* room_map);

    virtual void collect_special_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    void draw(GraphicsManager*);

private:
	void set_bound();
	void set_riding(Car*);

	Car* car_;
    RidingState state_;

	friend class RidingStateDelta;
};

#endif // PLAYER_H
