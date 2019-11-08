#ifndef PLAYER_H
#define PLAYER_H

#include "gameobject.h"
#include "delta.h"

class MoveProcessor;

enum class PlayerState {
    Free = 1,
    Bound = 2,
    RidingNormal = 3,
	RidingHidden = 4,
};

class Car;

class Player: public GameObject {
public:
    Player(Point3 pos, PlayerState state);
    virtual ~Player();

	void serialize(MapFileO& file);
	static std::unique_ptr<GameObject> deserialize(MapFileI& file);

	std::unique_ptr<GameObject> duplicate(RoomMap*, DeltaFrame*);

    std::string name();
    ObjCode obj_code();
	int color();

	FPoint3 cam_pos();
	
	void set_free(DeltaFrame*);
	void validate_state(RoomMap*, DeltaFrame*);

	void set_strictest(RoomMap*, DeltaFrame*);
	bool toggle_riding(RoomMap*, DeltaFrame*, MoveProcessor*);
	void destroy(MoveProcessor*, CauseOfDeath);
	void undestroy();
	CauseOfDeath death();

    Car* car_riding();
	Car* car_bound(RoomMap* map);

    virtual void collect_special_links(std::vector<GameObject*>& links);

    void draw(GraphicsManager*);

	PlayerState state();

	void set_car(Car* car);

	bool active_ = false;

private:
	void set_bound();

	Car* car_;
	PlayerState state_;
	CauseOfDeath death_ = CauseOfDeath::None;

	friend class PlayerStateDelta;
};


class PlayerStateDelta : public Delta {
public:
	PlayerStateDelta(Player* player);
	~PlayerStateDelta();
	void revert();

private:
	Player* player_;
	Car* car_;
	PlayerState state_;
	CauseOfDeath death_;
};

#endif // PLAYER_H
