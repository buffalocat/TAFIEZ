#pragma once

#include "objectmodifier.h"

class GameObject;
class RoomMap;
class MapFileI;
class MapFileO;
class DeltaFrame;

enum class ModCode;
enum class BlockTexture;

class PuppetBlock : public ObjectModifier {
public:
	PuppetBlock(GameObject* parent, RoomMap* room_map);
	virtual ~PuppetBlock();

	std::string name();
	ModCode mod_code();
	void serialize(MapFileO& file);
	static void deserialize(MapFileI&, RoomMap*, GameObject*);

	BlockTexture texture();

	void setup_on_put(RoomMap* room_map);
	void cleanup_on_take(RoomMap* room_map);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

private:
	RoomMap* map_;
};