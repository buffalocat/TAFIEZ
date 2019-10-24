#pragma once
#include "objectmodifier.h"
#include "delta.h"

class WorldResetKey : public ObjectModifier {
public:
	WorldResetKey(GameObject* parent, bool collected = false);
	~WorldResetKey();

	void make_str(std::string&);
	ModCode mod_code();

	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	void setup_on_put(RoomMap*, DeltaFrame*, bool real);

	void draw(GraphicsManager*, FPoint3);

protected:
	bool collected_;

	friend class KeyCollectDelta;
};


class KeyCollectDelta : public Delta {
public:
	KeyCollectDelta(WorldResetKey* key);
	~KeyCollectDelta();
	void revert();

private:
	WorldResetKey* key_;
};