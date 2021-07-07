#pragma once
#include "editortab.h"

class Room;

class RoomTab :	public EditorTab {
public:
	RoomTab(EditorState* editor);
	~RoomTab();

	void main_loop(EditorRoom*);
	void handle_left_click(EditorRoom*, Point3);

	void zone_options(Room*);
	void shift_extend_options(EditorRoom*);
};
