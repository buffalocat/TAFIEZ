#ifndef PUSHBLOCK_H
#define PUSHBLOCK_H

#include "gameobject.h"

class PushBlock: public ColoredBlock {
public:
    PushBlock(Point3 pos, int color, bool pushable, bool gravitable, Sticky sticky);
    virtual ~PushBlock();

    virtual std::string name();
    virtual ObjCode obj_code();
    virtual void serialize(MapFileO& file);
    static std::unique_ptr<GameObject> deserialize(MapFileI& file);

	std::unique_ptr<GameObject> duplicate(RoomMap*, DeltaFrame*);

    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links);
	bool has_sticky_neighbor(RoomMap*);

    virtual void draw(GraphicsManager*);

    Sticky sticky();

    Sticky sticky_;
};

#endif // PUSHBLOCK_H
