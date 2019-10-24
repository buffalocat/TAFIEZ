#pragma once

#include "objectmodifier.h"
#include "delta.h"

class ClearFlag : public ObjectModifier {
public:
	ClearFlag(GameObject* parent, int count, bool real, bool active, bool collected, char zone);
	~ClearFlag();

	void make_str(std::string&);
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	void setup_on_put(RoomMap*, DeltaFrame*, bool real);

	void draw(GraphicsManager*, FPoint3);

	bool real_;
	bool active_;
	bool collected_;

private:
	int count_;
	char zone_;
};


class ClearFlagToggleDelta : public Delta {
public:
	ClearFlagToggleDelta(ClearFlag* flag, RoomMap* map);
	~ClearFlagToggleDelta();
	void revert();

private:
	ClearFlag* flag_;
	RoomMap* map_;
};