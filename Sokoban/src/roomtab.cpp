#include "stdafx.h"
#include "roomtab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"
#include "gameobject.h"
#include "savefile.h"

RoomTab::RoomTab(EditorState* editor) : EditorTab(editor) {}

RoomTab::~RoomTab() {}

void RoomTab::main_loop(EditorRoom* eroom) {
	ImGui::Text("The Room Tab");
	ImGui::Separator();

	if (!eroom) {
		ImGui::Text("No room loaded.");
		return;
	}

	zone_options(eroom->map());
	
	int* req = &eroom->map()->clear_flag_req_;
	unsigned int* id = &eroom->map()->clear_id_;
	ImGui::InputInt("Clear Flag Requirement", req);
	clamp(req, 0, 255);
	editor_->manage_flag(*req > 0, id, eroom);

	ImGui::Text("Current Global ID: %u", *id);

	ImGui::Separator();

	shift_extend_options(eroom);
}

const int NUM_ZONES = 36;

int zone_input_callback(ImGuiInputTextCallbackData* data) {
	data->CursorPos = 2;
	data->SelectionStart = 2;
	data->SelectionEnd = 2;
	unsigned short* input_chars = ImGui::GetIO().InputCharacters;
	for (int i = 0; (i <= 16) && input_chars[i]; ++i) {
		if (input_chars[i] > 127) {
			continue;
		}
		char c = (char)input_chars[i];
		if (('a' <= c) && (c <= 'z')) {
			// Make uppercase
			c &= 95;
		}
		if ((('0' <= c) && (c <= '9')) || (('A' <= c) && (c <= 'Z'))) {
			data->Buf[0] = c;
			data->BufDirty = true;
			return 1;
		}
	}
	return 0;
}

void RoomTab::zone_options(RoomMap* room) {
	static char zone[3];
	zone[0] = room->zone_;
	zone[1] = ' ';
	ImGui::InputText("Zone##ROOM", zone, 3,
		ImGuiInputTextFlags_CallbackAlways,
		&zone_input_callback);
	room->zone_ = zone[0];
}

void RoomTab::shift_extend_options(EditorRoom* eroom) {
	Point3 cur_room_dims{ eroom->map()->width_, eroom->map()->height_, eroom->map()->depth_ };
	ImGui::Text("Current Room Dimensions: (%d,%d,%d)", cur_room_dims.x, cur_room_dims.y, cur_room_dims.z);

	static int shift_width;
	static int shift_height;
	static int shift_depth;

	ImGui::InputInt("Shift Width##ROOM", &shift_width);
	ImGui::InputInt("Shift Height##ROOM", &shift_height);
	ImGui::InputInt("Shift Depth##ROOM", &shift_depth);

	clamp(&shift_width, 1 - cur_room_dims.x, MAX_ROOM_DIMS - cur_room_dims.x);
	clamp(&shift_height, 1 - cur_room_dims.y, MAX_ROOM_DIMS - cur_room_dims.y);
	clamp(&shift_depth, 1 - cur_room_dims.z, MAX_ROOM_DIMS - cur_room_dims.z);

	if (ImGui::Button("Shift room?##ROOM")) {
		Point3 dpos = { shift_width, shift_height, shift_depth };
		eroom->room->shift_by(dpos);
		shift_width = 0;
		shift_height = 0;
		shift_depth = 0;
		eroom->start_pos += dpos;
		eroom->room->offset_pos_ += dpos;
		if (!eroom->map()->valid(eroom->start_pos)) {
			eroom->start_pos = { 0,0,0 };
		}
		eroom = editor_->reload(eroom);
	}

	static int extend_width;
	static int extend_height;
	static int extend_depth;

	ImGui::InputInt("Extend Width##ROOM", &extend_width);
	ImGui::InputInt("Extend Height##ROOM", &extend_height);
	ImGui::InputInt("Extend Depth##ROOM", &extend_depth);

	clamp(&extend_width, 1 - cur_room_dims.x, MAX_ROOM_DIMS - cur_room_dims.x);
	clamp(&extend_height, 1 - cur_room_dims.y, MAX_ROOM_DIMS - cur_room_dims.y);
	clamp(&extend_depth, 1 - cur_room_dims.z, MAX_ROOM_DIMS - cur_room_dims.z);

	if (ImGui::Button("Extend room?##ROOM")) {
		Point3 dpos = { extend_width, extend_height, extend_depth };
		eroom->room->extend_by(dpos);
		extend_width = 0;
		extend_height = 0;
		extend_depth = 0;
		if (!eroom->map()->valid(eroom->start_pos)) {
			eroom->start_pos = { 0,0,0 };
		}
		eroom = editor_->reload(eroom);
	}
}

void RoomTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
	RoomMap* map = eroom->map();
	if (!map->view(pos)) {
		auto player = map->view(eroom->start_pos);
		map->take_from_map(player, false, false, nullptr);
		player->pos_ = pos;
		eroom->start_pos = pos;
		map->put_in_map(player, false, false, nullptr);
	}
}