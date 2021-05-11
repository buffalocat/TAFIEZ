#ifndef ROOMMAP_H
#define ROOMMAP_H

#include "point.h"
#include "delta.h"

class GameState;
class PlayingState;
class GameObjectArray;
class Signaler;
class Effects;
class MapLayer;
class GraphicsManager;
class AnimationManager;
class TextRenderer;
class DeltaFrame;
class MoveProcessor;
class ObjectModifier;
class MapFileI;
class MapFileO;
class RoomMap;
class PlayingGlobalData;
class FrozenObject;

class GameObject;
class Player;

class Door;
class AutoBlock;
class PuppetBlock;
class ClearFlag;
class Car;

class PlayerCycle;

enum class ObjRefCode;

class RoomMap {
public:
    RoomMap(GameObjectArray& objs, GameState* state, int width, int height, int depth);
    virtual ~RoomMap();
    bool valid(Point3 pos);

    int& at(Point3);
    GameObject* view(Point3);

	virtual void push_to_object_array(std::unique_ptr<GameObject>, DeltaFrame*);
	void remove_from_object_array(GameObject*);
	void put_in_map(GameObject*, bool real, bool activate_listeners, DeltaFrame*);
    void take_from_map(GameObject*, bool real, bool activate_listeners, DeltaFrame*);
	virtual void create_in_map(std::unique_ptr<GameObject>, bool activate_listeners, DeltaFrame*);
	GameObject* deref_object(FrozenObject* frozen_obj);

	void create_wall(Point3);
	void clear(Point3);

    void shift(GameObject*, Point3, bool activate_listeners, DeltaFrame*);
    void batch_shift(std::vector<GameObject*>, Point3, bool activate_listeners, DeltaFrame*);

    void serialize(MapFileO& file, bool write_obj_ids);

    void draw(GraphicsManager*, double angle);
    void draw_layer(GraphicsManager*, int z);

    void shift_all_objects(Point3 d);
    void extend_by(Point3 d);
    void shift_by(Point3 d);

    void set_initial_state(PlayingState* playing_state);
    void reset_local_state();

	void initialize_animation(AnimationManager* anims);

    void initialize_automatic_snake_links();

    void push_signaler(std::unique_ptr<Signaler>);
    void check_signalers(DeltaFrame*, MoveProcessor*);
    void remove_signaler(Signaler*);

	void check_clear_flag_collected();
	void collect_flag(bool real);

	void validate_players(DeltaFrame*);

	void add_door(Door* door);
	void remove_door(Door* door);
	unsigned int get_unused_door_id();

	void remove_auto(AutoBlock* obj);
	void remove_puppet(PuppetBlock* obj);
	void remove_clear_flag(ClearFlag* obj);
	void remove_moved_car(Car* obj);

    void add_listener(ObjectModifier*, Point3);
    void remove_listener(ObjectModifier*, Point3);
    void activate_listeners_at(Point3);
    void activate_listener_of(ObjectModifier* obj);
    void alert_activated_listeners(DeltaFrame*, MoveProcessor*);
	void handle_moved_cars(MoveProcessor*);

	PlayingState* playing_state();

	PlayerCycle* player_cycle();

	std::vector<Door*>& door_group(unsigned int id);
	std::vector<Player*>& player_list();

// Public "private" members
    int width_;
    int height_;
    int depth_;

	std::vector<ClearFlag*> clear_flags_{};
	int clear_flag_req_ = 0;
	unsigned int clear_id_ = 0;
	char zone_ = '!';
	bool clear_flags_changed_ = false;

	std::string name_;
	bool inited_ = false;

	std::unique_ptr<PlayerCycle> player_cycle_;

	std::vector<AutoBlock*> autos_{};
	std::vector<PuppetBlock*> puppets_{};
	std::vector<Car*> moved_cars_{};

	std::map<unsigned int, std::vector<Door*>> door_groups_{};

    GameObjectArray& obj_array_;
	PlayingGlobalData* global_{};
	std::vector<MapLayer> layers_{};
	GameState* state_;

	TextRenderer* text_renderer();

	std::unordered_map<Point3, std::vector<ObjectModifier*>, Point3Hash> listeners_{};
	std::vector<std::unique_ptr<Signaler>> signalers_{};

	std::set<ObjectModifier*> activated_listeners_{};

    // For providing direct signaler access
    friend class SwitchTab;
};


// Deltas

class PutDelta : public Delta {
public:
	PutDelta(GameObject* obj);
	PutDelta(FrozenObject obj);
	~PutDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject obj_;
};


class TakeDelta : public Delta {
public:
	TakeDelta(GameObject* obj);
	TakeDelta(FrozenObject obj);
	~TakeDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject obj_;
};


class WallDestructionDelta : public Delta {
public:
	WallDestructionDelta(Point3 pos);
	~WallDestructionDelta();

	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	Point3 pos_;
};


class ObjArrayPushDelta : public Delta {
public:
	ObjArrayPushDelta(GameObject* obj);
	ObjArrayPushDelta(FrozenObject obj);
	~ObjArrayPushDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject obj_;
};


class MotionDelta : public Delta {
public:
	MotionDelta(GameObject* obj, Point3 dpos);
	MotionDelta(FrozenObject obj, Point3 dpos);
	~MotionDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject obj_;
	Point3 dpos_;
};


class BatchMotionDelta : public Delta {
public:
	BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos);
	BatchMotionDelta(std::vector<FrozenObject>&& objs, Point3 dpos);
	~BatchMotionDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	std::vector<FrozenObject> objs_;
	Point3 dpos_;
};


class ActivePlayerGuard {
public:
	ActivePlayerGuard(PlayerCycle*);
	~ActivePlayerGuard();
private:
	PlayerCycle* cycle_;
};

class PlayerCycle {
public:
	PlayerCycle();
	~PlayerCycle();

	void add_player(Player* player, DeltaFrame*, bool init);
	void add_player_at_pos(Player* player, int index);
	void set_active_player(Player* player);
	void remove_player(Player* player, DeltaFrame*);
	bool cycle_player(DeltaFrame*);
	Player* current_player();
	Player* dead_player();
	bool any_player_alive();
	void serialize_permutation(MapFileO & file);
	void deserialize_permutation(MapFileI& file, RoomMap* room_map);

private:
	std::vector<Player*> players_{};
	Player* dead_player_{};
	int index_ = -1;
	int dead_index_ = -1;

	friend class RoomMap;
	friend class ActivePlayerGuard;
	friend class AddPlayerDelta;
	friend class RemovePlayerDelta;
	friend class CyclePlayerDelta;
};


class AddPlayerDelta : public Delta {
public:
	AddPlayerDelta(int index);
	~AddPlayerDelta();

	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	int index_;
};


class RemovePlayerDelta : public Delta {
public:
	RemovePlayerDelta(Player*, Player* dead_player, int index, int dead_index, int rem);
	RemovePlayerDelta(FrozenObject player, FrozenObject dead_player, int index, int dead_index, int rem);
	~RemovePlayerDelta();

	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject player_;
	FrozenObject dead_player_;
	int index_;
	int dead_index_;
	int rem_;
};


class CyclePlayerDelta : public Delta {
public:
	CyclePlayerDelta(Player* dead_player, int index, int dead_index);
	CyclePlayerDelta(FrozenObject dead_player, int index, int dead_index);
	~CyclePlayerDelta();

	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject dead_player_;
	int index_;
	int dead_index_;
};


#endif // ROOMMAP_H
