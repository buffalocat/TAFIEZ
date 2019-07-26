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

    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links);

    virtual void draw(GraphicsManager*);
	void draw_force_indicators(GraphicsManager* gfx, FPoint3 p, float radius);

    Sticky sticky();

    Sticky sticky_;
};

#endif // PUSHBLOCK_H
