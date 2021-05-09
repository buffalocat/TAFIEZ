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
	PuppetBlock(GameObject* parent);
	virtual ~PuppetBlock();

	void make_str(std::string&);
	ModCode mod_code();

	static void deserialize(MapFileI&, GameObjectArray*, GameObject*);

	bool valid_parent(GameObject*);

	BlockTexture texture();

	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

private:
	RoomMap* map_;
};