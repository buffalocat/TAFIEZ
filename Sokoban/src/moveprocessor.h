#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "point.h"
#include "delta.h"

class Player;
class GameObject;
class PlayingState;
class AnimationManager;
class Room;
class RoomMap;
class DeltaFrame;
class SnakeBlock;
class Door;
class Switchable;
class Gate;
class Incinerator;


enum class MoveStep {
	Default = 0,
	Horizontal = 1,
	PreFallSwitch = 2,
	ColorChange = 3,
	Done = 4,
	DoorMove = 5,
	PostDoorInit = 6,
	FirstLoadInit = 7,
	ToggleRiding = 8,
	Jump = 9,
	Waiting = 10,
};

enum class MoveAction {
	None = 0,
	DoorExit = 1,
};

enum class DoorState {
	None = 0,
	AwaitingEntry = 1,
	AwaitingIntExit = 2,
	AwaitingExtExit = 3,
	AwaitingUnentry = 4,
	IntSucceeded = 5,
	ExtSucceeded = 6,
	Voided = 7,
};

struct DoorTravellingObj {
	GameObject* raw;
	Point3 rel_pos;
	DoorTravellingObj(GameObject* obj, Door* door);
};

class MoveProcessor {
public:
    MoveProcessor(PlayingState*, RoomMap*, DeltaFrame*, Player*, bool);
    ~MoveProcessor();

	bool update();

	void set_standing_door();
	void reset_player_jump();

    bool try_move_horizontal(Point3);
    bool try_color_change();
	bool try_toggle_riding();
	bool try_special_action();
	bool try_jump(bool can_rejump);
	void try_jump_refresh();
	bool try_toggle_grapple();

    void try_fall_step();
    void perform_switch_checks(bool skippable);
	void push_rising_gate(Gate* gate);

	void plan_door_move(Door*);
    void try_door_entry();
	void place_door_travelling_objects();
	void try_int_door_exit();
	void try_door_unentry();
	void ext_door_exit();

    void add_to_fall_check(GameObject*);
	void collect_adj_fall_checks(GameObject*);

	void set_initializer_state();

	PlayingState* playing_state_;
	AnimationManager* anims_;
	RoomMap* map_;
	DeltaFrame* delta_frame_;
	Player* player_;

	std::vector<GameObject*> moving_blocks_{};
	std::vector<GameObject*> fall_check_{};

	std::unordered_set<Switchable*> activated_switchables_{};

	std::vector<Incinerator*> alerted_incinerators_{};
	void run_incinerators();
	std::unordered_map<Point3, std::vector<Gate*>, Point3Hash> rising_gates_{};
	void raise_gates();

private:
	Door* standing_door_{};
	Door* entry_door_{};
	std::vector<DoorTravellingObj> door_travelling_objs_{};
	std::vector<Door*> exit_doors_{};
	Room* dest_room_{};

	unsigned int frames_{};
	MoveStep state_ = MoveStep::Default;
	MoveAction deferred_action_ = MoveAction::None;
	DoorState door_state_ = DoorState::None;

    bool animated_;
};


class RoomChangeDelta : public Delta {
public:
	RoomChangeDelta(Room* room);
	RoomChangeDelta(std::string room_name);
	~RoomChangeDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	std::string room_name_;
};


class ToggleGravitableDelta : public Delta {
public:
	ToggleGravitableDelta(GameObject* obj);
	ToggleGravitableDelta(FrozenObject obj);
	~ToggleGravitableDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject obj_;
};

class Car;

class ColorChangeDelta : public Delta {
public:
	ColorChangeDelta(Car* car, bool undo);
	ColorChangeDelta(FrozenObject car, bool undo);
	~ColorChangeDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject car_;
	bool undo_;
};

#endif // MOVEPROCESSOR_H
