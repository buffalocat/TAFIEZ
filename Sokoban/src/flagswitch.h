#ifndef FLAGSWITCH_H
#define FLAGSWITCH_H

#include "pressswitch.h"

class FlagSwitch : public PressSwitch {
public:
	FlagSwitch(GameObject* parent, bool active, int orientation);
	~FlagSwitch();

	void make_str(std::string&);
	ModCode mod_code();
	void serialize(MapFileO& file);
	static void deserialize(MapFileI&, RoomMap*, GameObject*);

	bool should_toggle(RoomMap*);
	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void draw(GraphicsManager*, FPoint3);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	int orientation_;
};

#endif //FLAGSWITCH_H