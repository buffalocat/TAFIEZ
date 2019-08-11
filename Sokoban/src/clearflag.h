#pragma once

#include "objectmodifier.h"

class ClearFlag : public ObjectModifier {
public:
	ClearFlag(GameObject* parent, bool real, bool split);
	~ClearFlag();

	std::string name();
	ModCode mod_code();
	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

private:
	bool real_;
	bool split_;
};