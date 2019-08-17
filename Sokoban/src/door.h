#ifndef DOOR_H
#define DOOR_H

#include "switchable.h"

class Room;
class RoomMap;
class GraphicsManager;
class MapFileI;
class MapFileO;

struct DoorData {
    Point3_S16 pos;
	std::string start;
    std::string dest;
    DoorData(Point3_S16 p, std::string start_room, std::string dest_room);
};

class Door: public Switchable {
public:
    Door(GameObject* parent, int count, bool persistent, bool def, bool active);
    virtual ~Door();
    Door(const Door&);

	void make_str(std::string&);
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    bool relation_check();
    void relation_serialize(MapFileO& file);
    bool can_set_state(bool state, RoomMap*);

    void set_data(Point3_S16, std::string start, std::string dest);
	void reset_data();
	DoorData* data();
	bool usable();

    void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void setup_on_put(RoomMap*, bool real);
	void cleanup_on_take(RoomMap*, bool real);

    void draw(GraphicsManager*, FPoint3);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

private:
    std::unique_ptr<DoorData> data_;
};

#endif // DOOR_H
