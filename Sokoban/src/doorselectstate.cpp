#include "stdafx.h"
#include "doorselectstate.h"

#include "gameobject.h"
#include "room.h"
#include "roommap.h"
#include "door.h"
#include "graphicsmanager.h"

DoorSelectState::DoorSelectState(GameState* parent, Room* room, Point3 cam_pos, unsigned int* exit_door_id):
EditorBaseState(parent), room_ {room}, cam_pos_ {cam_pos}, exit_door_id_{exit_door_id} {}

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

	RoomMap* room_map = room_->map();
    display_hover_pos_object(cam_pos_, room_map);

    ImGui::Separator();

    if (*exit_door_id_ == 0) {
        ImGui::Text("Destination not selected.");
    } else {
		for (auto* exit_door : room_map->door_group(*exit_door_id_)) {
			Point3 exit_pos = exit_door->pos();
			ImGui::Text("Destination Pos: (%d,%d,%d)", exit_pos.x, exit_pos.y, exit_pos.z);
			ImGui::Text(room_->map()->view(exit_pos)->to_str().c_str());
		}
    }
	ImGui::Text("Press escape to return.");
    ImGui::End();

	gfx_->set_state(GraphicsState::None);
	gfx_->pre_object_rendering();
	gfx_->draw_objects();
	gfx_->post_rendering();
}

void DoorSelectState::handle_left_click(Point3 pos) {
	if (GameObject* door_obj = room_->map()->view(pos)) {
		if (auto door = dynamic_cast<Door*>(door_obj->modifier())) {
			*exit_door_id_ = door->door_id_;
		}
	}
}

void DoorSelectState::handle_right_click(Point3 pos) {}
