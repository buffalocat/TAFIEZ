#pragma once

#include "editortab.h"
#include "point.h"

class CameraContext;
enum class CameraCode;

class CameraTab : public EditorTab {
public:
	CameraTab(EditorState*, GraphicsManager*);
	~CameraTab();
	void init();
	void main_loop(EditorRoom*);
	void handle_left_click(EditorRoom*, Point3);
	void handle_right_click(EditorRoom*, Point3);

	int get_context_labels(const char* labels[], std::vector<std::unique_ptr<CameraContext>>& contexts);
	void camera_type_choice(CameraCode* cam_code_ptr);

	void normalize_a_b();

private:
	int xa, ya, xb, yb;
	int x_, y_, w_, h_, priority_;
	float rad_, tilt_, rot_;
	int xpad_, ypad_;
	FPoint3 center_;
};
