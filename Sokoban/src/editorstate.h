#pragma once

#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include "editorbasestate.h"
#include "editortab.h"
#include "room.h"

class RoomMap;
class EditorGlobalData;
class GameObjectArray;

struct EditorRoom {
    std::unique_ptr<Room> room;
    Point3 start_pos;
    Point3 cam_pos;
    bool changed = true;
	bool include_car;
    EditorRoom(std::unique_ptr<Room>, Point3, bool inc_car);
	void write_to_file(MapFileO& file);

    RoomMap* map();
    std::string name();
};

class EditorState: public EditorBaseState {
public:
    EditorState(GameState* parent);
    virtual ~EditorState();
    void main_loop();
	bool can_quit(bool confirm);

	bool handle_keyboard_input_main_state();
    void set_active_room(std::string name);
    int get_room_names(const char* room_names[]);
    EditorRoom* get_room(std::string name);

    void new_room(std::string name, int width, int height, int depth);
    bool load_room(std::string name, bool from_main);
	void load_room_from_path(std::filesystem::path path);
    void save_room(EditorRoom* eroom, bool commit);
    EditorRoom* reload(EditorRoom* eroom);
	void load_save_cycle();
    void unload_current_room();
    void commit_current_room();
    void commit_all();

	void set_active_tab_by_index(int i);
    void begin_test();

	void manage_flag(bool create, unsigned int* flag_ptr, EditorRoom* eroom);

	EditorRoom* active_room_{};
	EditorTab* active_tab_{};
	std::unique_ptr<EditorGlobalData> global_ = std::make_unique<EditorGlobalData>();

private:
	std::map<std::string, std::unique_ptr<EditorRoom>> rooms_{};
	std::vector<std::pair<std::string, std::unique_ptr<EditorTab>>> tabs_{};

    std::unique_ptr<GameObjectArray> objs_ = std::make_unique<GameObjectArray>();

    void handle_left_click(Point3);
    void handle_right_click(Point3);
};

#endif // EDITORSTATE_H
