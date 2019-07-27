#pragma once

#include "editortab.h"
#include "point.h"

class CameraContext;

class CameraTab : public EditorTab {
	CameraTab(EditorState*, GraphicsManager*);
	~CameraTab();
	void init();
	void main_loop(EditorRoom*) = 0;
	void handle_left_click(EditorRoom*, Point3);
	void handle_right_click(EditorRoom*, Point3);

	int get_context_labels(const char* labels[], std::vector<std::unique_ptr<CameraContext>>& contexts);

private:
	int x_, y_, w_, h_, priority_;
	float rad_, tilt_, rot_;
	int xpad_, ypad_;
	Point3 center_;
};