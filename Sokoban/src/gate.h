#ifndef GATE_H
#define GATE_H

#include "switchable.h"


class GateBody;
class GraphicsManager;
class MapFileO;

class Gate: public Switchable {
public:
    Gate(GameObject* parent, GateBody* body, int color, int count, bool persistent, bool def, bool active, bool waiting);
    virtual ~Gate();

	void make_str(std::string&);
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    void shift_internal_pos(Point3 d);

    void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
    void collect_special_links(RoomMap*, std::vector<GameObject*>&);

    bool can_set_state(bool state, RoomMap*);
    void apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*);
	void raise_gate(RoomMap*, DeltaFrame*);

	void cleanup_on_take(RoomMap* map, bool real);
	void setup_on_put(RoomMap* map, bool real);
	void destroy(MoveProcessor*, CauseOfDeath, bool collect_links);
	void undestroy();

    void draw(GraphicsManager*, FPoint3);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

    int color_;
    GateBody* body_;
};

#endif // GATE_H
