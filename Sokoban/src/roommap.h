#ifndef ROOMMAP_H
#define ROOMMAP_H

#include "point.h"

class GameObjectArray;
class Signaler;
class Effects;
class MapLayer;
class GraphicsManager;
class DeltaFrame;
class MoveProcessor;
class ObjectModifier;
class MapFileO;
class RoomMap;
class PlayingGlobalData;

class GameObject;
class SnakeBlock;
class AutoBlock;
class PuppetBlock;
class ClearFlag;

class RoomMap {
public:
    RoomMap(GameObjectArray& objs, PlayingGlobalData* global, GraphicsManager* gfx, int width, int height, int depth);
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

	void set_initial_state_on_start();
	void set_initial_state_after_door(DeltaFrame* delta_frame, MoveProcessor* mp);
	void set_initial_state_in_editor();
    void set_initial_state(bool editor_mode, DeltaFrame* delta_frame, MoveProcessor* mp);
    void reset_local_state();

    void initialize_automatic_snake_links();

    void push_signaler(std::unique_ptr<Signaler>);
    void check_signalers(DeltaFrame*, MoveProcessor*);
    void remove_signaler(Signaler*);

	void check_clear_flag_collected(DeltaFrame*);
	void collect_flag();
	void uncollect_flag(int req);

	void remove_auto(AutoBlock* obj);
	void remove_puppet(PuppetBlock* obj);

    void add_listener(ObjectModifier*, Point3);
    void remove_listener(ObjectModifier*, Point3);
    void activate_listeners_at(Point3);
    void activate_listener_of(ObjectModifier* obj);
    void alert_activated_listeners(DeltaFrame*, MoveProcessor*);

    void make_fall_trail(GameObject*, int height, int drop);

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

	std::vector<AutoBlock*> autos_{};
	std::vector<PuppetBlock*> puppets_{};

    GameObjectArray& obj_array_;
	PlayingGlobalData* global_;
	GraphicsManager* gfx_;
	std::vector<MapLayer> layers_{};

private:
	std::unordered_map<Point3, std::vector<ObjectModifier*>, Point3Hash> listeners_{};
	std::vector<std::unique_ptr<Signaler>> signalers_{};

	std::set<ObjectModifier*> activated_listeners_{};

    // TODO: find more appropriate place for this
    std::unique_ptr<Effects> effects_ = std::make_unique<Effects>();


    // For providing direct signaler access
    friend class SwitchTab;
};

#endif // ROOMMAP_H
