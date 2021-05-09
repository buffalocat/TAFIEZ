#ifndef AUTOBLOCK_H
#define AUTOBLOCK_H

#include "objectmodifier.h"

class AutoBlock: public ObjectModifier {
public:
    AutoBlock(GameObject* parent);
    virtual ~AutoBlock();

	void make_str(std::string&);
    ModCode mod_code();
    static void deserialize(MapFileI&, GameObjectArray*, GameObject*);

	bool valid_parent(GameObject*);

	BlockTexture texture();

	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

private:
    // Auto needs to be able to alert the map when it is duplicated/destroyed
    RoomMap* map_;
};

#endif // AUTOBLOCK_H
