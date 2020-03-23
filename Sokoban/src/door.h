#ifndef DOOR_H
#define DOOR_H

#include "switchable.h"

class Room;
class RoomMap;
class GraphicsManager;
class MapFileI;
class MapFileO;

struct DoorData {
	std::string start;
    std::string dest;
	unsigned int id;
    DoorData(std::string start_room, std::string dest_room, unsigned int id);
};

class Door: public Switchable {
public:
    Door(GameObject* parent, int count, bool persistent, bool def, bool active, unsigned int door_id);
    virtual ~Door();
    Door(const Door&);

	void make_str(std::string&);
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    bool relation_check();
    void relation_serialize(MapFileO& file);
    bool can_set_state(bool state, RoomMap*);

    void set_data(unsigned int door_id, std::string start, std::string dest);
	void reset_data();
	DoorData* data();
	bool usable();

    void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
	void apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*);

	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	void destroy(MoveProcessor*, CauseOfDeath);
	void signal_animation(AnimationManager*, DeltaFrame*);

    void draw(GraphicsManager*, FPoint3);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	unsigned int door_id_;
	unsigned int map_flag_ = 0;
	
	std::unique_ptr<DoorData> data_{};
};

#endif // DOOR_H
