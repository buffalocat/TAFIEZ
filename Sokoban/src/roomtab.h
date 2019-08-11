#pragma once
#include "editortab.h"

class RoomMap;

class RoomTab :	public EditorTab {
public:
	RoomTab(EditorState* editor, GraphicsManager* gfx);
	~RoomTab();

	void main_loop(EditorRoom*);
	void handle_left_click(EditorRoom*, Point3);

	void zone_options(RoomMap*);
	void shift_extend_options(EditorRoom*);
};
