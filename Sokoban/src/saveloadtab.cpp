#include "stdafx.h"
#include "saveloadtab.h"

#include "common_constants.h"
#include "room.h"
#include "roommap.h"
#include "gameobject.h"
#include "editorstate.h"
#include "player.h"

SaveLoadTab::SaveLoadTab(EditorState* editor) : EditorTab(editor) {}

SaveLoadTab::~SaveLoadTab() {}

void SaveLoadTab::main_loop(EditorRoom* eroom) {
	ImGui::Text("The Save/Load Tab");

	map_load_and_create();

	ImGui::Separator();

	loaded_rooms_listbox();

	if (ImGui::Button("Save All Maps##SAVELOAD")) {
		editor_->commit_all();
	}

	if (!eroom) {
		if (ImGui::Button("Load/Save Cycle All Maps in Main")) {
			editor_->load_save_cycle();
		}
		return;
	}

	ImGui::Separator();

	if (ImGui::Button("Save Current Map##SAVELOAD")) {
		editor_->commit_current_room();
	}

	if (ImGui::Button("Begin Test Session##SAVELOAD")) {
		editor_->begin_test();
	}
}


void SaveLoadTab::map_load_and_create() {
	const int MAX_MAP_NAME_SIZE = 256;
	static char map_name_input[MAX_MAP_NAME_SIZE] = "";

	ImGui::InputText(".map##SAVELOAD", map_name_input, IM_ARRAYSIZE(map_name_input), ImGuiInputTextFlags_CharsNoBlank);
	ImGui::SameLine();
	if (ImGui::Button("...##SAVELOAD_file_name")) {
		ImGui::OpenPopup("SAVELOAD_file_name");
	}
	if (ImGui::BeginPopup("SAVELOAD_file_name")) {
		file_choice(MAPS_MAIN, map_name_input, MAX_MAP_NAME_SIZE);
		ImGui::EndPopup();
	}
	if (ImGui::Button("Load Map##SAVELOAD")) {
		if (editor_->load_room(map_name_input, true)) {
			editor_->set_active_room(map_name_input);
			return;
		} else {
			std::cout << "Failed to load room." << std::endl;
		}
	}

	ImGui::Separator();

	static int width = DEFAULT_BOARD_WIDTH;
	static int height = DEFAULT_BOARD_HEIGHT;
	static int depth = DEFAULT_BOARD_DEPTH;
	ImGui::InputInt("Room Width##SAVELOAD", &width);
	ImGui::InputInt("Room Height##SAVELOAD", &height);
	ImGui::InputInt("Room Depth##SAVELOAD", &depth);

	clamp(&width, 1, MAX_ROOM_DIMS);
	clamp(&height, 1, MAX_ROOM_DIMS);
	clamp(&depth, 1, MAX_ROOM_DIMS);

	if (ImGui::Button("Create New Map##SAVELOAD")) {
		editor_->new_room(map_name_input, width, height, depth);
	}
}

void SaveLoadTab::loaded_rooms_listbox() {
	static int current = 0;
	const char* room_names[256];
	int len = editor_->get_room_names(room_names);
	if (ImGui::ListBox("Loaded Maps##SAVELOAD", &current, room_names, len, len)) {
		editor_->set_active_room(std::string(room_names[current]));
	}
}