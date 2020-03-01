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
class MapFileO;
class RoomMap;
class PlayingGlobalData;

class GameObject;
class Player;

class Door;
class AutoBlock;
class PuppetBlock;
class ClearFlag;
class Car;

class PlayerCycle;

class RoomMap {
public:
    RoomMap(GameObjectArray& objs, GameState* state, int width, int height, int depth);
    ~RoomMap();
    bool valid(Point3 pos);

    int& at(Point3);
    GameObject* view(Point3);

	void push_to_object_array(std::unique_ptr<GameObject>, DeltaFrame*);
	void remove_from_object_array(GameObject*);
	void put_in_map(GameObject*, bool real, bool activate_listeners, DeltaFrame*);
    void take_from_map(GameObject*, bool real, bool activate_listeners, DeltaFrame*);
	void create_in_map(std::unique_ptr<GameObject>, bool activate_listeners, DeltaFrame*);

	void create_wall(Point3);
	void clear(Point3);

    void shift(GameObject*, Point3, bool activate_listeners, DeltaFrame*);
    void batch_shift(std::vector<GameObject*>, Point3, bool activate_listeners, DeltaFrame*);

    void serialize(MapFileO& file);

    void draw(GraphicsManager*, double angle);
    void draw_layer(GraphicsManager*, int z);

    void shift_all_objects(Point3 d);
    void extend_by(Point3 d);
    void shift_by(Point3 d);

	void set_initial_state_on_start(PlayingState* state);
	void set_initial_state_after_door(DeltaFrame* delta_frame, MoveProcessor* mp);
	void set_initial_state_in_editor();
    void set_initial_state(bool editor_mode, DeltaFrame* delta_frame, MoveProcessor* mp);
    void reset_local_state();

	void initialize_animation(AnimationManager* anims);

    void initialize_automatic_snake_links();

    void push_signaler(std::unique_ptr<Signaler>);
    void check_signalers(DeltaFrame*, MoveProcessor*);
    void remove_signaler(Signaler*);

	void check_clear_flag_collected(DeltaFrame*);
	void collect_flag();
	void uncollect_flag(int req);

	void free_unbound_players(DeltaFrame*);

	void add_door(Door* door);
	void remove_door(Door* door);
	unsigned int get_unused_door_id();

	void remove_auto(AutoBlock* obj);
	void remove_puppet(PuppetBlock* obj);

    void add_listener(ObjectModifier*, Point3);
    void remove_listener(ObjectModifier*, Point3);
    void activate_listeners_at(Point3);
    void activate_listener_of(ObjectModifier* obj);
    void alert_activated_listeners(DeltaFrame*, MoveProcessor*);
	void handle_moved_cars(MoveProcessor*);

    void make_fall_trail(GameObject*, int height, int drop);

	std::vector<Door*>& door_group(unsigned int id);
	std::vector<Player*>& player_list();

// Public "private" members
    int width_;
    int height_;
    int depth_;

	std::map<ClearFlag*, bool> clear_flags_{};
	int clear_flag_req_ = 0;
	unsigned int clear_id_ = 0;
	char zone_ = '!';
	bool clear_flags_changed_ = false;

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

private:
	std::unordered_map<Point3, std::vector<ObjectModifier*>, Point3Hash> listeners_{};
	std::vector<std::unique_ptr<Signaler>> signalers_{};

	std::set<ObjectModifier*> activated_listeners_{};

    // TODO: find more appropriate place for this
    std::unique_ptr<Effects> effects_ = std::make_unique<Effects>();


    // For providing direct signaler access
    friend class SwitchTab;
};

// Deltas

class PutDelta : public Delta {
public:
	PutDelta(GameObject* obj, RoomMap* map);
	~PutDelta();
	void revert();

private:
	GameObject* obj_;
	RoomMap* map_;
};


class TakeDelta : public Delta {
public:
	TakeDelta(GameObject* obj, RoomMap* map);
	~TakeDelta();
	void revert();

private:
	GameObject* obj_;
	RoomMap* map_;
};


class WallDestructionDelta : public Delta {
public:
	WallDestructionDelta(Point3 pos, RoomMap* map);
	~WallDestructionDelta();

	void revert();

private:
	Point3 pos_;
	RoomMap* map_;
};


class ObjArrayPushDelta : public Delta {
public:
	ObjArrayPushDelta(GameObject* obj, RoomMap* map);
	~ObjArrayPushDelta();
	void revert();

private:
	GameObject* obj_;
	RoomMap* map_;
};


class MotionDelta : public Delta {
public:
	MotionDelta(GameObject* obj, Point3 dpos, RoomMap* map);
	~MotionDelta();
	void revert();

private:
	GameObject* obj_;
	Point3 dpos_;
	RoomMap* map_;
};


class BatchMotionDelta : public Delta {
public:
	BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos, RoomMap* map);
	~BatchMotionDelta();
	void revert();

private:
	std::vector<GameObject*> objs_;
	Point3 dpos_;
	RoomMap* map_;
};


class ClearFlagCollectionDelta : public Delta {
public:
	ClearFlagCollectionDelta(RoomMap* map, int req);
	~ClearFlagCollectionDelta();
	void revert();

private:
	RoomMap* map_;
	int req_;
};


class MapInitDelta : public Delta {
public:
	MapInitDelta(RoomMap* map);
	~MapInitDelta();

	void revert();

private:
	RoomMap* map_;
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
	AddPlayerDelta(PlayerCycle*, int index);
	~AddPlayerDelta();

	void revert();

private:
	PlayerCycle* cycle_;
	int index_;
};


class RemovePlayerDelta : public Delta {
public:
	RemovePlayerDelta(PlayerCycle*, Player*, Player* dead_player, int index, int dead_index, int rem);
	~RemovePlayerDelta();

	void revert();

private:
	PlayerCycle* cycle_;
	Player* player_;
	Player* dead_player_;
	int index_;
	int dead_index_;
	int rem_;
};


class CyclePlayerDelta : public Delta {
public:
	CyclePlayerDelta(PlayerCycle*, Player* dead_player, int index, int dead_index);
	~CyclePlayerDelta();

	void revert();

private:
	PlayerCycle* cycle_;
	Player* dead_player_;
	int index_;
	int dead_index_;
};


#endif // ROOMMAP_H
