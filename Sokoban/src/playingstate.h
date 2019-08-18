#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include "gamestate.h"
#include "common_constants.h"

class GameObjectArray;
class GraphicsManager;
class FontManager;
class Room;
class RoomMap;
class GameObject;
class Player;
class MoveProcessor;
class PlayingGlobalData;

class UndoStack;
class DeltaFrame;
class DoorMoveDelta;

struct Point3;

class Door;
struct DoorTravellingObj;

enum class PSTask {
	None,
	Save,
	WorldReset,
	Quit,
};

struct PlayingRoom {
	std::unique_ptr<Room> room;
	bool changed = true;
	PlayingRoom(std::unique_ptr<Room>);
};

class PlayingState: public GameState {
public:
    PlayingState(GameState* parent);
    virtual ~PlayingState();

    void main_loop();
    void handle_input();
	void create_move_processor();
	
	Room* active_room();
	bool activate_room(Room*);
    bool activate_room(std::string);
    void load_room_from_path(std::filesystem::path path, bool use_default_player);
	virtual bool load_room(std::string name, bool use_default_player) = 0;

    bool can_use_door(Door*, std::vector<DoorTravellingObj>&, Room**);

	void snap_camera_to_player();

	virtual void make_subsave();
	virtual void world_reset();

	std::unique_ptr<PlayingGlobalData> global_{ std::make_unique<PlayingGlobalData>() };

protected:
	std::map<std::string, std::unique_ptr<PlayingRoom>> loaded_rooms_{};
	Room* room_{};
	Player* player_{};
	std::unique_ptr<GameObjectArray> objs_ = std::make_unique<GameObjectArray>();
	std::unique_ptr<UndoStack> undo_stack_{ std::make_unique<UndoStack>(MAX_UNDO_DEPTH) };

private:
	std::unique_ptr<MoveProcessor> move_processor_{};
	std::unique_ptr<DeltaFrame> delta_frame_{};

	PSTask queued_task_ = PSTask::None;

    friend DoorMoveDelta;
};

#endif // PLAYINGSTATE_H
