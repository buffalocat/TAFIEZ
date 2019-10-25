#include "stdafx.h"
#include "doortab.h"

#include "room.h"
#include "roommap.h"
#include "gameobject.h"
#include "door.h"
#include "editorstate.h"
#include "doorselectstate.h"

DoorTab::DoorTab(EditorState* editor): EditorTab(editor) {}

DoorTab::~DoorTab() {}

void DoorTab::init() {
    ent_door_ = nullptr;
    exit_room_ = nullptr;
    exit_door_id_ = 0;
}

static bool failed_to_load_room = false;

void print_door_data(RoomMap* map, unsigned int id) {
	for (auto* exit_door : map->door_group(id)) {
		Point3 exit_pos = exit_door->pos();
		ImGui::Text("Destination Pos: (%d,%d,%d)", exit_pos.x, exit_pos.y, exit_pos.z);
		ImGui::Text(map->view(exit_pos)->to_str().c_str());
	}
}

void DoorTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Door Tab");
    ImGui::Separator();
    if (!eroom) {
        ImGui::Text("No room loaded.");
        return;
    }

    ImGui::Text("Click on a door to select it.");
    if (!ent_door_) {
        return;
    }

    ImGui::Separator();
	
	RoomMap* room_map = eroom->map();
    Point3 pos = ent_door_->pos();
    ImGui::Text("Currently selected door: (%d,%d,%d)", pos.x, pos.y, pos.z);
    DoorData* data = ent_door_->data();
    if (data) {
		ImGui::Text("Destination Room: %s", data->dest.c_str());
		if (EditorRoom* exit_room = editor_->get_room(data->dest)) {
			print_door_data(exit_room->map(), data->id);
		} else {
			ImGui::Text("...but that room's .map file doesn't appear to exist.");
		}
		if (ImGui::Button("Reset the destination to nothing?##DOOR")) {
			ent_door_->reset_data();
		}
    } else {
        ImGui::Text("This door doesn't have a destination yet");
    }

	ImGui::Separator();

	ImGui::Text("Right click a door to select it as the destination.");

    static int current = 0;
    const char* room_names[256];
    int len = editor_->get_room_names(room_names);
    if (ImGui::ListBox("Loaded Maps##DOOR", &current, room_names, len, len)) {
        if (exit_room_ != editor_->get_room(room_names[current])) {
            exit_room_ = editor_->get_room(room_names[current]);
            exit_door_id_ = 0;
        }
    }

    if (!exit_room_) {
        ImGui::Text("No room selected");
        return;
	} else if (exit_room_ == eroom) {
		ImGui::Text("That's the current room");
	} else {
		ImGui::Text(("Destination Room: " + exit_room_->name()).c_str());

		if (ImGui::Button("Select Destination Position##DOOR")) {
			Point3 start_pos = exit_room_->start_pos;
			auto select_state = std::make_unique<DoorSelectState>(editor_, exit_room_->room.get(), start_pos, &exit_door_id_);
			editor_->create_child(std::move(select_state));
		}
	}

    if (exit_door_id_ == 0) {
        ImGui::Text("No destination selected");
        return;
    }

	print_door_data(exit_room_->map(), exit_door_id_);

	if (ImGui::Button("Link One Way##DOOR")) {
		ent_door_->set_data(exit_door_id_, eroom->name(), exit_room_->name());
	}

	if (ImGui::Button("Link Both Ways##DOOR")) {
		ent_door_->set_data(exit_door_id_, eroom->name(), exit_room_->name());
		for (auto* exit_door : exit_room_->map()->door_group(exit_door_id_)) {
			exit_door->set_data(ent_door_->door_id_, exit_room_->name(), eroom->name());
		}
		exit_room_->changed = true;
	}
}

void DoorTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    if (GameObject* obj = eroom->map()->view(pos)) {
        if (Door* door = dynamic_cast<Door*>(obj->modifier())) {
            ent_door_ = door;
            DoorData* data = door->data();
            if (data) {
                // Try to load the destination room
                exit_room_ = editor_->get_room(data->dest);
                if (!exit_room_) {
                    editor_->load_room(data->dest, true);
                    exit_room_ = editor_->get_room(data->dest);
                }
                
            }
        }
    }
}

void DoorTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
	if (GameObject* obj = eroom->map()->view(pos)) {
		if (Door* door = dynamic_cast<Door*>(obj->modifier())) {
			exit_room_ = eroom;
			exit_door_id_ = door->door_id_;
		}
	}
}
