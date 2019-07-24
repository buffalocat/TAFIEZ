#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include <map>

#include "gamestate.h"

class GameObjectArray;
class GraphicsManager;
class Room;
class RoomMap;
class GameObject;
class Player;
class MoveProcessor;

class UndoStack;
class DeltaFrame;
class DoorMoveDelta;

struct Point3;

class Door;
struct DoorTravellingObj;

struct PlayingRoom {
	std::unique_ptr<Room> room;
	bool changed;
	PlayingRoom(std::unique_ptr<Room>);

	RoomMap* map();
	std::string name();
};

class PlayingState: public GameState {
public:
    PlayingState();
    virtual ~PlayingState();
    void main_loop();
    void handle_input();
	Room* active_room();
	bool activate_room(Room*);
    bool activate_room(const std::string&);
    virtual bool load_room(const std::string&, bool use_default_player) = 0;

    bool can_use_door(Door*, std::vector<DoorTravellingObj>&, Room**);

	void snap_camera_to_player();

protected:
    std::map<std::string, std::unique_ptr<PlayingRoom>> loaded_rooms_;
    Room* room_;
    Player* player_;
	std::unique_ptr<GameObjectArray> objs_;

private:
    std::unique_ptr<MoveProcessor> move_processor_;
    std::unique_ptr<UndoStack> undo_stack_;
    std::unique_ptr<DeltaFrame> delta_frame_;

    friend DoorMoveDelta;
};

#endif // PLAYINGSTATE_H
