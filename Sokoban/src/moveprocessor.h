#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "point.h"

class Player;
class GameObject;
class PlayingState;
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
	Point3 dest;
};

class MoveProcessor {
public:
    MoveProcessor(PlayingState*, RoomMap*, DeltaFrame*, Player*, bool);
    ~MoveProcessor();

	bool update();
	void abort();

	void reset_player_jump();

    bool try_move_horizontal(Point3);
    bool try_color_change();
	bool try_toggle_riding();
	bool try_jump();
	void try_jump_refresh();

    void try_fall_step();
    void perform_switch_checks(bool skippable);
	void push_rising_gate(Gate* gate);

	void plan_door_move(Door*);
    void try_door_entry();
	void try_int_door_exit();
	void try_door_unentry();
	void ext_door_exit();

    void add_to_fall_check(GameObject*);
	void add_neighbors_to_fall_check(GameObject*);
    void add_to_moving_blocks(GameObject*);

	void set_initializer_state();

	PlayingState* playing_state_;
	RoomMap* map_;
	DeltaFrame* delta_frame_;
	Player* player_;

	std::vector<GameObject*> moving_blocks_{};
	std::vector<GameObject*> fall_check_{};

	std::vector<Switchable*> activated_switchables_{};

	std::vector<Incinerator*> activated_incinerators_{};
	void run_incinerators();
	std::unordered_map<Point3, std::vector<Gate*>, Point3Hash> rising_gates_{};
	void raise_gates();

private:
	Door* entry_door_{};
	std::vector<DoorTravellingObj> door_travelling_objs_{};
	Room* dest_room_{};

	unsigned int frames_{};
	MoveStep state_ = MoveStep::Default;
	DoorState door_state_ = DoorState::None;

    bool animated_;
};

#endif // MOVEPROCESSOR_H
