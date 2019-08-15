#include "stdafx.h"
#include "doorselectstate.h"

#include "gameobject.h"
#include "room.h"
#include "roommap.h"
#include "door.h"

DoorSelectState::DoorSelectState(Room* room, Point3 cam_pos, Point3* exit_pos):
EditorBaseState(), room_ {room}, cam_pos_ {cam_pos}, exit_pos_ {exit_pos} {}

DoorSelectState::~DoorSelectState() {}

void DoorSelectState::main_loop() {
    bool p_open = true;
    if (!ImGui::Begin("Door Destination Select Window##DOOR", &p_open)) {
        ImGui::End();
        return;
    }

    handle_mouse_input(cam_pos_, room_);
	if (keyboard_cooldown_ > 0) {
		--keyboard_cooldown_;
	} else if (!want_capture_keyboard()) {
		if (handle_keyboard_input(cam_pos_, room_)) {
			keyboard_cooldown_ = MAX_COOLDOWN;
		}
	}

    room_->draw_at_pos(cam_pos_, false, true, one_layer_);

    display_hover_pos_object(cam_pos_, room_->map());

    ImGui::Separator();

    if (exit_pos_->x == -1) {
        ImGui::Text("Destination not selected.");
    } else {
        ImGui::Text("Destination Pos: (%d,%d,%d)", exit_pos_->x, exit_pos_->y, exit_pos_->z);
        if (GameObject* obj = room_->map()->view(*exit_pos_)) {
            ImGui::Text(obj->to_str().c_str());
        } else {
            ImGui::Text("Empty");
        }
    }
    ImGui::Text("Press escape to return.");

    ImGui::End();
}

void DoorSelectState::handle_left_click(Point3 pos) {
    if (pos.x == -1) {
        return;
    }
    *exit_pos_ = pos;
}

void DoorSelectState::handle_right_click(Point3 pos) {}
