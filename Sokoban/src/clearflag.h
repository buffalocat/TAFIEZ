#pragma once

#include "objectmodifier.h"

class ClearFlag : public ObjectModifier {
public:
	ClearFlag(GameObject* parent, int count, bool real, bool active, bool collected, char zone);
	~ClearFlag();

	std::string name();
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void cleanup_on_take(RoomMap* map, bool real);
	void setup_on_put(RoomMap* map, bool real);

	void draw(GraphicsManager*, FPoint3);

	bool real_;
	bool active_;
	bool collected_;

private:
	int count_;
	char zone_;
};