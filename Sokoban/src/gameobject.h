#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "common_enums.h"
#include "point.h"
#include "colorcycle.h"

class ObjectModifier;
class PositionalAnimation;
class DeltaFrame;
class RoomMap;
class GraphicsManager;
class MapFileI;
class MapFileO;

struct Component;
struct PushComponent;
struct FallComponent;

// Base class of all objects that occupy a tile in a RoomMap
class GameObject {
public:
    virtual ~GameObject();
    GameObject(const GameObject&);

    std::string to_str();
    virtual std::string name() = 0;

    virtual ObjCode obj_code() = 0;
    virtual bool skip_serialization();
    virtual void serialize(MapFileO& file);
    virtual bool relation_check();
    virtual void relation_serialize(MapFileO& file);

    Point3 shifted_pos(Point3 d);
    void shift_internal_pos(Point3 d);
    void abstract_shift(Point3 dpos, DeltaFrame* delta_frame);
	void abstract_put(Point3 pos, DeltaFrame* delta_frame);

    virtual void draw(GraphicsManager*) = 0;

    virtual void setup_on_put(RoomMap*, bool real);
    virtual void cleanup_on_take(RoomMap*, bool real);

    void set_modifier(std::unique_ptr<ObjectModifier> mod);

	PushComponent* push_comp();
	FallComponent* fall_comp();

	void collect_sticky_component(RoomMap*, Sticky, Component*);
	virtual Sticky sticky();
	virtual bool has_sticky_neighbor(RoomMap*);
	virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links);
	virtual void collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>& links);

	ObjectModifier* modifier();
	virtual int color();

    void reset_animation();
    void set_linear_animation(Point3);
    bool update_animation(); // Return whether the animation is done
    void shift_pos_from_animation();
    FPoint3 real_pos();

	std::unique_ptr<ObjectModifier> modifier_{};
	std::unique_ptr<PositionalAnimation> animation_{};
	Component* comp_;
    Point3 pos_;
	unsigned int id_ = 0;
    bool pushable_;
    bool gravitable_;
	// Is the object physically in the map?
	bool tangible_ = false;
	// Temporary flag for active cars/autos/puppets
	bool driven_ = false;

protected:
    GameObject(Point3 pos, bool pushable, bool gravitable);
};

// TODO: add copy constructors?
class Block : public GameObject {
public:
	Block(Point3 pos, bool pushable, bool gravitable);
	virtual ~Block();
};

// A block to which the player can be "bound" - it's most things that aren't Walls.
// The common superclass of PushBlock and SnakeBlock (will there be more...?)
class ColoredBlock : public Block {
public:
	ColoredBlock(Point3 pos, int color, bool pushable, bool gravitable);
	virtual ~ColoredBlock();

	int color();
	bool has_sticky_neighbor(RoomMap*);

	int color_ = 0;
};

#endif // GAMEOBJECT_H
