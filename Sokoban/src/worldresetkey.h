#pragma once
#include "objectmodifier.h"

class WorldResetKey : public ObjectModifier {
public:
	WorldResetKey(GameObject* parent, bool collected = false);
	~WorldResetKey();

	std::string name();
	ModCode mod_code();

	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void cleanup_on_take(RoomMap* map, bool real);
	void setup_on_put(RoomMap* map, bool real);

	void draw(GraphicsManager*, FPoint3);

protected:
	bool collected_;

	friend class CollectibleDelta;
};

