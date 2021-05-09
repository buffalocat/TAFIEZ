#ifndef GATE_H
#define GATE_H

#include "switchable.h"


class GateBody;
class GraphicsManager;
class MapFileO;

const int MAX_GATE_ANIMATION_FRAMES = 4;

enum class GateAnimationState {
	None,
	Raise,
	Lower,
};

class Gate: public Switchable {
public:
    Gate(GameObject* parent, GateBody* body, int color, int count, bool persistent, bool def, bool active, bool waiting);
    virtual ~Gate();

	void make_str(std::string&);
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, GameObjectArray*, GameObject*);

	GameObject* get_subordinate_object();

    void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
    void collect_special_links(std::vector<GameObject*>&);

    bool can_set_state(bool state, RoomMap*);
    void apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*);
	void raise_gate(RoomMap*, DeltaFrame*, MoveProcessor* mp);
	
	bool update_animation(PlayingState*);
	void start_raise_animation();
	void start_lower_animation();

	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void destroy(MoveProcessor*, CauseOfDeath);
	void undestroy();

    void draw(GraphicsManager*, FPoint3);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

    int color_;
    GateBody* body_;
	
	GateAnimationState animation_state_ = GateAnimationState::None;
	int animation_time_ = 0;
};

#endif // GATE_H
