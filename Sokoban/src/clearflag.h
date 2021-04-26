#pragma once

#include "objectmodifier.h"
#include "delta.h"

class ClearFlag : public ObjectModifier {
public:
	ClearFlag(GameObject* parent, bool real, bool active, bool collected);
	~ClearFlag();

	void make_str(std::string&);
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void destroy(MoveProcessor*, CauseOfDeath);
	void signal_animation(AnimationManager*, DeltaFrame*);

	int color();
	void draw(GraphicsManager*, FPoint3);

	bool real_;
	bool active_;
	bool collected_;
};


class ClearFlagToggleDelta : public Delta {
public:
	ClearFlagToggleDelta(ClearFlag* flag);
	~ClearFlagToggleDelta();
	void serialize(MapFileO&, GameObjectArray*);
	void revert(RoomMap*);

private:
	FrozenObject flag_;
	RoomMap* map_;
};