#ifndef OBJECTMODIFIER_H
#define OBJECTMODIFIER_H

#include "common_enums.h"
#include "point.h"
#include "delta.h"

class GameObject;
class RoomMap;
class DeltaFrame;
class MoveProcessor;
class MapFileI;
class MapFileO;
class GraphicsManager;
class AnimationManager;
class PlayingState;
class PlayingGlobalData;
class EditorGlobalData;
class Room;
class GameObjectArray;

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
	virtual void relation_serialize_frozen(MapFileO& file);
	virtual std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*) = 0;

	virtual bool valid_parent(GameObject*);
	GameObject* parent_;

	virtual GameObject* get_subordinate_object();

	virtual void draw(GraphicsManager*, FPoint3);

    Point3 pos();
    Point3 shifted_pos(Point3 d);
    Point3 pos_above(); // A convenience function often needed by Modifiers
    bool pushable();
    bool gravitable();

	virtual BlockTexture texture();

    virtual void setup_on_put(RoomMap*, DeltaFrame*, bool real);
    virtual void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	virtual void setup_on_editor_creation(EditorGlobalData* global, Room* room);
	virtual void cleanup_on_editor_destruction(EditorGlobalData* global);
	virtual void destroy(MoveProcessor*, CauseOfDeath);
	virtual void undestroy();
	virtual void signal_animation(AnimationManager*, DeltaFrame*);
	virtual bool update_animation(PlayingState*);
	virtual void reset_animation();

    // Every type of Modifier can have at most one callback function for map listeners
    virtual void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
    virtual void collect_special_links(std::vector<GameObject*>&);
};


class ModDestructionDelta : public Delta {
public:
	ModDestructionDelta(ObjectModifier* obj);
	ModDestructionDelta(FrozenObject mod);
	~ModDestructionDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileIwithObjs& file);

private:
	FrozenObject mod_;
};


#endif // OBJECTMODIFIER_H
