#pragma once

#include "objectmodifier.h"

class ClearFlag : public ObjectModifier {
public:
	ClearFlag(GameObject* parent, int count, bool real, char zone);
	~ClearFlag();

	std::string name();
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void draw(GraphicsManager*, FPoint3);

	bool real_;

private:
	int count_;
	char zone_;
};