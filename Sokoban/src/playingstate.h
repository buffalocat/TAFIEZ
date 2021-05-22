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
class GlobalAnimation;
class AutosavePanel;

class UndoStack;
class DeltaFrame;

struct Point3;

class Door;
struct DoorTravellingObj;

enum class CauseOfDeath;
enum class SaveType;

enum class DeathState {
	Alive,
	DeadAlone,
	DeadCanSwitch,
};

struct PlayingRoom {
	std::unique_ptr<Room> room;
	bool changed = true;
	PlayingRoom(std::unique_ptr<Room>);
};

class KeyStatus {
public:
	KeyStatus(int code);
	void update(GameState* state);
	void consume();

	int status_;

private:
	int code_;
	bool press_;
	bool held_;
};

class PlayingState : public GameState {
public:
	PlayingState(GameState* parent, PlayingGlobalData* global);
	virtual ~PlayingState();

	virtual void main_loop();
	void update_key_status();
	void handle_input();
	void create_move_processor(Player* player);
	void update_global_animation();

	Room* active_room();
	bool activate_room(Room*);
	bool activate_room(std::string);
	void load_room_from_path(std::filesystem::path path, bool use_default_player);
	virtual bool load_room(std::string name, bool use_default_player) = 0;
	virtual void make_subsave(SaveType type, unsigned int save_index = 0, AutosavePanel* panel = nullptr);

	bool can_use_door(Door* ent_door, Room** dest_room_ptr, std::vector<DoorTravellingObj>& objs, std::vector<Door*>& exit_doors);

	void move_camera_to_player(bool snap);

	Player* player_doa();

	bool mandatory_wait_ = false;

	PlayingGlobalData* global_{};
	std::unique_ptr<AnimationManager> anims_{};
	std::unique_ptr<GlobalAnimation> global_anim_{};

	Room* room_{};
	std::unique_ptr<DeltaFrame> delta_frame_{};

	std::map<std::string, std::unique_ptr<PlayingRoom>> loaded_rooms_{};
	AutosavePanel* queued_autosave_ = nullptr;

	std::unique_ptr<GameObjectArray> objs_ = std::make_unique<GameObjectArray>();
	std::unique_ptr<UndoStack> undo_stack_{ std::make_unique<UndoStack>(this, MAX_UNDO_DEPTH) };
	std::unique_ptr<MoveProcessor> move_processor_{};

private:
	void handle_escape();

	std::unique_ptr<StringDrawer> death_message_{};
	std::unique_ptr<StringDrawer> death_submessage_{};
	void set_death_text();

	int input_cooldown = 0;
	int undo_combo = 0;
	KeyStatus car_ride_key_{ GLFW_KEY_X };
	KeyStatus color_change_key_{ GLFW_KEY_C };
	KeyStatus player_switch_key_{ GLFW_KEY_V };
	KeyStatus restart_key_{ GLFW_KEY_R };
	int restart_cooldown_ = 60;
	CauseOfDeath current_death_;
	DeathState current_death_state_;
	bool can_toggle_fullscreen_ = false;
};

#endif // PLAYINGSTATE_H
