#ifndef PRESSSWITCH_H
#define PRESSSWITCH_H


#include "objectmodifier.h"
#include "switch.h"

class MapFileI;
class MapFileO;
class RoomMap;
class DeltaFrame;
class GraphicsManager;

class PressSwitch: public Switch {
public:
    PressSwitch(GameObject* parent, int color, bool persistent, bool active);
    virtual ~PressSwitch();

	virtual void make_str(std::string&);
    virtual ModCode mod_code();
    virtual void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    virtual void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

    bool check_send_signal(RoomMap*, DeltaFrame*);
    bool should_toggle(RoomMap*);

    void setup_on_put(RoomMap*, DeltaFrame*, bool real);
    void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);

    virtual void draw(GraphicsManager*, FPoint3);

    virtual std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

    int color_;
};

#endif // PRESSSWITCH_H
