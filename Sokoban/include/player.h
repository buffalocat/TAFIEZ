#ifndef PLAYER_H
#define PLAYER_H

#include "gameobject.h"

class Car;

class Player: public GameObject {
public:
    Player(Point3 pos, RidingState state);
    virtual ~Player();

    std::string name();
    ObjCode obj_code();
    bool skip_serialization();

    bool is_agent();

    void toggle_riding(RoomMap* room_map, DeltaFrame*);
    Car* get_car(RoomMap* room_map, bool strict);

    virtual void collect_special_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    void draw(GraphicsManager*);

    RidingState state_;
};

#endif // PLAYER_H
