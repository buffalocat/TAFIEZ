#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H

#include "gamestate.h"
#include "common_constants.h"

class GameObjectArray;
class GraphicsManager;
class AnimationManager;
class FontManager;
class StringDrawer;
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

enum class CauseOfDeath;


struct PlayingRoom {
	std::unique_ptr<Room> room;
	bool changed = true;
	PlayingRoom(std::unique_ptr<Room>);
};

class PlayingState : public GameState {
public:
	PlayingState(GameState* parent, PlayingGlobalData* global);
	virtual ~PlayingState();

	void main_loop();
	void handle_input();
	void create_move_processor(Player* player);

	Room* active_room();
	bool activate_room(Room*);
	bool activate_room(std::string);
	void load_room_from_path(std::filesystem::path path, bool use_default_player);
	virtual bool load_room(std::string name, bool use_default_player) = 0;

	bool can_use_door(Door* ent_door, Room** dest_room_ptr, std::vector<DoorTravellingObj>& objs, std::vector<Door*>& exit_doors);

	void move_camera_to_player(bool snap);

	Player* player_doa();

	virtual void make_subsave();
	virtual void world_reset();

	bool mandatory_wait_ = false;

	PlayingGlobalData* global_{};
	std::unique_ptr<AnimationManager> anims_{};

	Room* room_{};
	std::unique_ptr<DeltaFrame> delta_frame_{};

protected:
	std::map<std::string, std::unique_ptr<PlayingRoom>> loaded_rooms_{};
	std::unique_ptr<GameObjectArray> objs_ = std::make_unique<GameObjectArray>();
	std::unique_ptr<UndoStack> undo_stack_{ std::make_unique<UndoStack>(MAX_UNDO_DEPTH) };
	std::unique_ptr<MoveProcessor> move_processor_{};

private:
	std::unique_ptr<StringDrawer> death_message_{};
	std::unique_ptr<StringDrawer> death_submessage_{};
	void set_death_text();

    friend DoorMoveDelta;
};

#endif // PLAYINGSTATE_H
