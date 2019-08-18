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
class GateBody;

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

    bool try_move(Point3);
    bool try_color_change();
	bool try_toggle_riding();

    void try_fall_step();
    void perform_switch_checks(bool skippable);

	void plan_door_move(Door*);
    void try_door_entry();
	void try_int_door_exit();
	void try_door_unentry();
	void ext_door_exit();

    void add_to_fall_check(GameObject*);
	void add_neighbors_to_fall_check(GameObject*);
    void add_to_moving_blocks(GameObject*);

    bool update();
    void abort();

	void set_initializer_state();

private:
    void move_bound(Point3);
    void move_general(Point3);

	std::vector<GameObject*> moving_blocks_{};
	std::vector<GameObject*> fall_check_{};

    PlayingState* playing_state_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
	Player* player_;

	Door* entry_door_{};
	std::vector<DoorTravellingObj> door_travelling_objs_{};
	Room* dest_room_{};

	unsigned int frames_{};
	MoveStep state_ = MoveStep::Default;
	DoorState door_state_ = DoorState::None;

    bool animated_;
};

#endif // MOVEPROCESSOR_H
