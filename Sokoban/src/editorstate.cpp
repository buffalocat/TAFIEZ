#include "stdafx.h"
#include "editorstate.h"

#include "savefile.h"
#include "gameobjectarray.h"
#include "graphicsmanager.h"
#include "gamestate.h"
#include "testplayingstate.h"
#include "gameobject.h"
#include "player.h"
#include "car.h"
#include "mapfile.h"
#include "room.h"
#include "roommap.h"
#include "camera.h"

#include "saveloadtab.h"
#include "roomtab.h"
#include "objecttab.h"
#include "doortab.h"
#include "switchtab.h"
#include "modifiertab.h"
#include "snaketab.h"
#include "cameratab.h"


#define INIT_TAB(NAME)\
tabs_.push_back(std::make_pair(#NAME, std::make_unique<NAME ## Tab>(this, gfx)));

EditorRoom::EditorRoom(std::unique_ptr<Room> arg_room, Point3 pos):
room {std::move(arg_room)}, start_pos {pos}, cam_pos {pos} {}

RoomMap* EditorRoom::map() {
    return room->map();
}

std::string EditorRoom::name() {
    return room->name();
}

EditorState::EditorState(GraphicsManager* gfx): EditorBaseState() {
    INIT_TAB(SaveLoad);
	INIT_TAB(Room);
    INIT_TAB(Object);
	INIT_TAB(Modifier);
	INIT_TAB(Switch);
    INIT_TAB(Door);
	INIT_TAB(Camera);
	INIT_TAB(Snake);
    active_tab_ = tabs_[0].second.get();
	global_->load_flags("maps");
}

#undef INIT_TAB

EditorState::~EditorState() {
	global_->save_flags("maps");
}

// These shortcuts are specific to EditorState, and *not* other classes
// which inherit from EditorBaseState
bool EditorState::handle_keyboard_input_main_state() {
	for (int i = 0; i < tabs_.size(); ++i) {
		if (glfwGetKey(window_, GLFW_KEY_1 + i) == GLFW_PRESS) {
			set_active_tab_by_index(i);
			return false;
		}
	}
	if (glfwGetKey(window_, GLFW_KEY_T)) {
		begin_test();
		return true;
	}
	return false;
}

void EditorState::main_loop() {
    bool p_open = true;
    if (!ImGui::Begin("Editor Window##ROOT", &p_open)) {
        ImGui::End();
        return;
    }

	if (keyboard_cooldown_ > 0) {
		--keyboard_cooldown_;
	} else if (!want_capture_keyboard()) {
		if (handle_keyboard_input_main_state()) {
			keyboard_cooldown_ = MAX_COOLDOWN;
		} else if (active_tab_->handle_keyboard_input()) {
			keyboard_cooldown_ = MAX_COOLDOWN;
		}
	}

	// Draw the editor tabs
	for (int i = 0; i < tabs_.size(); ++i) {
		auto& p = tabs_[i];
		if (ImGui::Button((p.first + "##ROOT").c_str())) {
			set_active_tab_by_index(i);
		} ImGui::SameLine();
	}
	ImGui::Text(""); //This consumes the stray SameLine from the loop

    if (active_room_) {
		// Header Information
        ImGui::Text(("Current Room: " + active_room_->room->name()).c_str());
        ImGui::Text("Current Height: %d", active_room_->cam_pos.z);
        if (one_layer_) {
            ImGui::Text("Only Showing This Layer (F to toggle)");
        } else {
            ImGui::Text("Showing Neighboring Layers (F to toggle)");
        }
		// Draw the active room
        active_room_->changed = true;
        active_room_->room->draw_at_pos(gfx_, active_room_->cam_pos, false, ortho_cam_, one_layer_);
		// Handle input
		handle_mouse_input(active_room_->cam_pos, active_room_->room.get());
		if (keyboard_cooldown_ == 0 && !want_capture_keyboard()) {
			if (handle_keyboard_input(active_room_->cam_pos, active_room_->room.get())) {
				keyboard_cooldown_ = MAX_COOLDOWN;
			}
		}
	}	

	// Draw the EditorTab
	ImGui::BeginChild("Active Tab Pane##ROOT", ImVec2(0, 0), true);
	active_tab_->main_loop(active_room_);
	ImGui::EndChildFrame();

	ImGui::End();
}

void EditorState::set_active_tab_by_index(int i) {
	active_tab_ = tabs_[i].second.get();
	active_tab_->init();
}

void EditorState::set_active_room(std::string name) {
    active_room_ = rooms_[name].get();
}

int EditorState::get_room_names(const char* room_names[]) {
    int i = 0;
    for (auto& p : rooms_) {
        room_names[i] = p.first.c_str();
        ++i;
    }
    return i;
}

EditorRoom* EditorState::get_room(std::string name) {
    if (rooms_.count(name)) {
        return rooms_[name].get();
    }
    return nullptr;
}

void EditorState::new_room(std::string name, int width, int height, int depth) {
    if (!name.size()) {
        std::cout << "Room name must be non-empty!" << std::endl;
        return;
    }
    if (rooms_.count(name)) {
        std::cout << "A room with that name is already loaded!" << std::endl;
        return;
    }
    auto room = std::make_unique<Room>(name);
    room->initialize(*objs_, nullptr, width, height, depth);
	Point3 player_pos{ 0,0,2 };
    room->map()->create_in_map(std::make_unique<Player>(player_pos, RidingState::Free), nullptr);
	room->set_cam_pos(player_pos, player_pos);
    rooms_[name] = std::make_unique<EditorRoom>(std::move(room), player_pos);
    set_active_room(name);
}

bool EditorState::load_room(std::string name, bool from_main) {
    std::filesystem::path path;
    if (from_main) {
        path = (MAPS_MAIN / name).concat(".map");
    } else {
        path = (MAPS_TEMP / name).concat(".map");
    }
    if (!std::filesystem::exists(path)) {
		return false;
    }
	load_room_from_path(path);
    return true;
}

void EditorState::load_room_from_path(std::filesystem::path path) {
	MapFileI file{ path };
	std::string name = path.stem().string();
	std::unique_ptr<Room> room = std::make_unique<Room>(name);

	Player* player = nullptr;
	room->load_from_file(*objs_, file, nullptr, &player);
	player->set_free();
	Point3 start_pos = player->pos_;
	room->map()->set_initial_state(true);
	room->set_cam_pos(start_pos, start_pos);
	rooms_[name] = std::make_unique<EditorRoom>(std::move(room), start_pos);
}

void EditorState::save_room(EditorRoom* eroom, bool commit) {
    std::filesystem::path path;
    if (commit) {
        path = (MAPS_MAIN / eroom->name()).concat(".map");
    } else {
		path = (MAPS_TEMP / eroom->name()).concat(".map");
    }
    MapFileO file{path};
	RoomMap* map = eroom->map();
	Player* player = dynamic_cast<Player*>(map->view(eroom->start_pos));
	player->set_strictest(map);
    eroom->room->write_to_file(file);
	player->set_free();
}

EditorRoom* EditorState::reload(EditorRoom* eroom) {
    std::string name = eroom->name();
    save_room(eroom, false);
    load_room(name, false);
    set_active_room(name);
    return active_room_;
}

void EditorState::load_save_cycle() {
	for (auto& path : std::filesystem::directory_iterator(MAPS_MAIN)) {
		load_room_from_path(path);
	}
	commit_all();
	defer_to_parent();
}

void EditorState::unload_current_room() {
    if (!active_room_) {
        return;
    }
    rooms_.erase(active_room_->room->name());
    active_room_ = nullptr;
}

void EditorState::commit_current_room() {
    if (!active_room_) {
        return;
    }
    save_room(active_room_, true);
}

void EditorState::commit_all() {
    for (auto& p : rooms_) {
        save_room(p.second.get(), true);
    }
}

void EditorState::begin_test() {
    if (!active_room_) {
        return;
    }
    for (auto& p : rooms_) {
        if (p.second->changed) {
            p.second->changed = false;
            save_room(p.second.get(), false);
        }
    }
    auto playing_state = std::make_unique<TestPlayingState>(active_room_->room->name());
    create_child(std::move(playing_state));
}

void EditorState::handle_left_click(Point3 pos) {
    active_tab_->handle_left_click(active_room_, pos);
}

void EditorState::handle_right_click(Point3 pos) {
    active_tab_->handle_right_click(active_room_, pos);
}
