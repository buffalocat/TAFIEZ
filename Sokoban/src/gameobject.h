#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "common_enums.h"
#include "point.h"
#include "colorcycle.h"
#include "delta.h"
#include "mapfile.h"

class ObjectModifier;
class PositionalAnimation;
class DeltaFrame;
class RoomMap;
class MoveProcessor;
class GraphicsManager;
class ModelInstancer;
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

	virtual GameObject* realize(PlayingState* state);
	virtual void realize_references(RoomMap* room_map);

    Point3 shifted_pos(Point3 d);
    void abstract_shift(Point3 dpos);
	void abstract_put(Point3 pos, DeltaFrame* delta_frame);

    virtual void setup_on_put(RoomMap*, DeltaFrame*, bool real);
    virtual void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	virtual void destroy(MoveProcessor*, CauseOfDeath);
	virtual void undestroy();

	virtual std::unique_ptr<GameObject> duplicate(RoomMap* , DeltaFrame* ) = 0;

    void set_modifier(std::unique_ptr<ObjectModifier> mod);

	PushComponent* push_comp();
	FallComponent* fall_comp();

	void collect_sticky_component(RoomMap*, Sticky, Component*);
	virtual Sticky sticky();
	virtual bool is_snake();
	virtual bool has_sticky_neighbor(RoomMap*);
	virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links);
	virtual void collect_special_links(std::vector<GameObject*>& links);

	GameObject* get_subordinate_object();

	ObjectModifier* modifier();
	virtual int color();
	FPoint3 real_pos();

	virtual void draw(GraphicsManager*) = 0;
	virtual void draw_squished(GraphicsManager*, FPoint3 p, float scale);

	std::unique_ptr<ObjectModifier> modifier_{};
	Component* comp_;
    Point3 pos_;
	FPoint3 dpos_{};
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

enum class ObjRefCode;


// TODO: add copy constructors?
class Block : public GameObject {
public:
	Block(Point3 pos, bool pushable, bool gravitable);
	virtual ~Block();

	void draw_force_indicators(ModelInstancer& model, FPoint3 p, double radius);
};

// A block to which the player can be "bound" - it's most things that aren't Walls.
// The common superclass of PushBlock and SnakeBlock (will there be more...?)
class ColoredBlock : public Block {
public:
	ColoredBlock(Point3 pos, int color, bool pushable, bool gravitable);
	virtual ~ColoredBlock();

	int color();

	int color_ = 0;
};


class DestructionDelta : public Delta {
public:
	DestructionDelta(GameObject* obj);
	DestructionDelta(FrozenObject obj);
	virtual ~DestructionDelta();
	void serialize(MapFileO&, GameObjectArray*);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileIwithObjs& file);

protected:
	FrozenObject obj_;
};


class AbstractShiftDelta : public Delta {
public:
	AbstractShiftDelta(GameObject* obj, Point3 dpos);
	AbstractShiftDelta(FrozenObject obj, Point3 dpos);
	~AbstractShiftDelta();
	void serialize(MapFileO&, GameObjectArray*);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileIwithObjs& file);

private:
	FrozenObject obj_;
	Point3 dpos_;
};


class AbstractPutDelta : public Delta {
public:
	AbstractPutDelta(GameObject* obj, Point3 pos);
	AbstractPutDelta(FrozenObject obj, Point3 pos);
	~AbstractPutDelta();
	void serialize(MapFileO&, GameObjectArray*);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileIwithObjs& file);

private:
	FrozenObject obj_;
	Point3 pos_;
};

#endif // GAMEOBJECT_H
