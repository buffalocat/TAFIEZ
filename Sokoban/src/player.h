#ifndef PLAYER_H
#define PLAYER_H

#include "gameobject.h"

class MoveProcessor;

enum class PlayerState {
    Free = 1,
    Bound = 2,
    RidingNormal = 3,
	RidingHidden = 4,
	Dead = 5,
};

class Car;

class Player: public GameObject {
public:
    Player(Point3 pos, PlayerState state);
    virtual ~Player();

	void serialize(MapFileO& file);
	static std::unique_ptr<GameObject> deserialize(MapFileI& file);

    std::string name();
    ObjCode obj_code();
	int color();

	FPoint3 cam_pos();
	
	void set_free(DeltaFrame* delta_frame);
	void validate_state(RoomMap* map);

	void set_strictest(RoomMap* map, DeltaFrame*);
	bool toggle_riding(RoomMap* map, DeltaFrame*, MoveProcessor* mp);
	void destroy(DeltaFrame*);

    Car* car_riding();
	Car* car_bound(RoomMap* map);

    virtual void collect_special_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    void draw(GraphicsManager*);

	PlayerState state();

private:
	void set_bound();
	void set_car(Car* car);

	Car* car_;
	PlayerState state_;

	friend class PlayerStateDelta;
};

#endif // PLAYER_H
