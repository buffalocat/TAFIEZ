#ifndef OBJECTMODIFIER_H
#define OBJECTMODIFIER_H

#include "common_enums.h"
#include "point.h"

class GameObject;
class RoomMap;
class DeltaFrame;
class MoveProcessor;
class MapFileI;
class MapFileO;
class GraphicsManager;
class PlayingGlobalData;
class EditorGlobalData;
class Room;

enum class BlockTexture;

// Base class of object modifiers such as Car, Door, Switch, and Gate
class ObjectModifier {
public:
    ObjectModifier(GameObject* parent);
    virtual ~ObjectModifier();

    virtual void make_str(std::string&) = 0;
    virtual ModCode mod_code() = 0;
	virtual void serialize(MapFileO& file);
    virtual bool relation_check();
    virtual void relation_serialize(MapFileO& file);
	virtual std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*) = 0;

	virtual bool valid_parent(GameObject*);
	GameObject* parent_;

	virtual void draw(GraphicsManager*, FPoint3);

    Point3 pos();
    Point3 shifted_pos(Point3 d);
    virtual void shift_internal_pos(Point3 d);
    Point3 pos_above(); // A convenience function often needed by Modifiers
    bool pushable();
    bool gravitable();

	virtual BlockTexture texture();

    virtual void setup_on_put(RoomMap*, bool real);
    virtual void cleanup_on_take(RoomMap*, bool real);
	virtual void setup_on_editor_creation(EditorGlobalData* global, Room* room);
	virtual void cleanup_on_editor_destruction(EditorGlobalData* global);
	virtual void destroy(DeltaFrame*, CauseOfDeath death);

    // Every type of Modifier can have at most one callback function for map listeners
    virtual void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
    virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&);
};

#endif // OBJECTMODIFIER_H
