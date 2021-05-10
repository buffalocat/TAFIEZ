#ifndef PUSHBLOCK_H
#define PUSHBLOCK_H

#include "gameobject.h"

enum class BlockTexture;

class PushBlock: public ColoredBlock {
public:
    PushBlock(Point3 pos, int color, bool pushable, bool gravitable, Sticky sticky);
    virtual ~PushBlock();

    virtual std::string name();
    virtual ObjCode obj_code();
    virtual void serialize(MapFileO& file);
    static std::unique_ptr<GameObject> deserialize(MapFileI& file, Point3 pos);

	std::unique_ptr<GameObject> duplicate(RoomMap*, DeltaFrame*);

    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links);
	bool has_sticky_neighbor(RoomMap*);

    virtual void draw(GraphicsManager*);
    void draw_squished(GraphicsManager*, FPoint3 p, float scale);

    Sticky sticky();
	BlockTexture get_block_texture();

    Sticky sticky_;
};

#endif // PUSHBLOCK_H
