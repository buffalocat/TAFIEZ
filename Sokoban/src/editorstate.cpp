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


EditorRoom::EditorRoom(std::unique_ptr<Room> arg_room, Point3 pos, bool inc_car):
	room{ std::move(arg_room) }, start_pos{ pos }, cam_pos{ pos }, include_car{ inc_car } {}

RoomMap* EditorRoom::map() {
    return room->map();
}

std::string EditorRoom::name() {
    return room->name();
}

void EditorRoom::write_to_file(MapFileO& file) {
	room->write_to_file(file);
	file << MapCode::DefaultPlayerPos;
	file << start_pos;
	if (include_car) {
		if (GameObject* obj_below = map()->view(start_pos - Point3{ 0, 0, 1 })) {
			if (Car* car = dynamic_cast<Car*>(obj_below->modifier())) {
				file << MapCode::DefaultCarPos;
				file << car->pos();
			}
		}
	}
	file << MapCode::End;
}

#define INIT_TAB(NAME)\
tabs_.push_back(std::make_pair(#NAME, std::make_unique<NAME ## Tab>(this)));

EditorState::EditorState(GameState* parent): EditorBaseState(parent) {
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
		if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
			if (glfwGetKey(window_, GLFW_KEY_1 + i) == GLFW_PRESS) {
				set_active_tab_by_index(i);
				return false;
			}
		}
	}
	if (glfwGetKey(window_, GLFW_KEY_T)) {
		begin_test();
		return true;
	}
	if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
		glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
		if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			commit_all();
		} else {
			commit_current_room();
		}
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

	if (keyboard_cooldown_ == 0 && !want_capture_keyboard()) {
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
		RoomMap* map = active_room_->map();
        ImGui::Text("Current Room: %s (%d,%d,%d)", active_room_->room->name().c_str(), map->width_, map->height_, map->depth_);
		unsigned int layer = active_room_->cam_pos.z;
		if (one_layer_) {
			ImGui::Text("Just Layer %d", layer);
		} else {
			ImGui::Text("Multiple Layers (%d)", layer);
		}
		display_hover_pos_object(active_room_->cam_pos, active_room_->map());
		// Draw the active room
        active_room_->changed = true;
        active_room_->room->draw_at_pos(active_room_->cam_pos, false, ortho_cam_, one_layer_);
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

	if (keyboard_cooldown_ > 0) {
		--keyboard_cooldown_;
	}

	gfx_->set_state(GraphicsState::None);
	gfx_->pre_rendering();
	gfx_->draw_objects();
	gfx_->post_rendering();
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
    auto room = std::make_unique<Room>(this, name);
    room->initialize(*objs_, width, height, depth);
	Point3 player_pos{ 0,0,2 };
    room->map()->create_in_map(std::make_unique<Player>(player_pos, PlayerState::Free), false, nullptr);
	room->set_cam_pos(player_pos, player_pos, false, true);
    rooms_[name] = std::make_unique<EditorRoom>(std::move(room), player_pos, true);
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
	std::unique_ptr<Room> room = std::make_unique<Room>(this, name);

	RoomInitData init_data;
	room->load_from_file(*objs_, file, global_.get(), &init_data);
	Player* player = init_data.default_player;
	player->set_free(nullptr);
	Point3 start_pos = player->pos_;
	room->map()->set_initial_state_in_editor();
	room->set_cam_pos(start_pos, start_pos, false, true);
	rooms_[name] = std::make_unique<EditorRoom>(std::move(room), start_pos, init_data.default_car != nullptr);
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
	player->set_strictest(map, nullptr);
    eroom->write_to_file(file);
	player->set_free(nullptr);
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
	queue_quit();
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
		if (p.second->changed) {
			p.second->changed = false;
			save_room(p.second.get(), true);
		}
    }
	if (active_room_) {
		active_room_->changed = true;
	}
}

void EditorState::begin_test() {
    if (!active_room_) {
        return;
    }
	save_room(active_room_, false);
    auto playing_state_unique = std::make_unique<TestPlayingState>(this);
	auto playing_state = playing_state_unique.get();
    create_child(std::move(playing_state_unique));
	playing_state->init(active_room_->room->name());
}

void EditorState::handle_left_click(Point3 pos) {
    active_tab_->handle_left_click(active_room_, pos);
}

void EditorState::handle_right_click(Point3 pos) {
    active_tab_->handle_right_click(active_room_, pos);
}

void EditorState::manage_flag(bool create, unsigned int* flag_ptr, EditorRoom* eroom) {
	if (create && *flag_ptr == 0) {
		unsigned int flag = global_->generate_flag();
		*flag_ptr = flag;
		global_->assign_flag(flag, eroom->name());
	} else if (!create && *flag_ptr > 0) {
		global_->destroy_flag(*flag_ptr);
		*flag_ptr = 0;
	}
}

bool EditorState::can_quit(bool confirm) {
	if (!confirm) {
		return true;
	}
	bool result = false;
	ImGui::OpenPopup("Quit?");
	if (ImGui::BeginPopupModal("Quit?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Are you sure you want to quit?\n");
		ImGui::Separator();
		if (ImGui::Button("Yes", ImVec2(-1, 0))) {
			ImGui::CloseCurrentPopup();
			result = true;
		}
		ImGui::EndPopup();
	}
	return result;
}