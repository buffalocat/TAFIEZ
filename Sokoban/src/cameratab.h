#pragma once

#include "editortab.h"
#include "point.h"

class CameraContext;
enum class CameraCode;

class CameraTab : public EditorTab {
public:
	CameraTab(EditorState*);
	~CameraTab();
	void init();
	void main_loop(EditorRoom*);
	void handle_left_click(EditorRoom*, Point3);
	void handle_right_click(EditorRoom*, Point3);

	int get_context_labels(const char* labels[], std::vector<std::unique_ptr<CameraContext>>& contexts);

	void normalize_rect_a_b();
	void normalize_vis_a_b();

private:
	std::string label_;
	IntRect rect_;
	int priority_;
	bool named_area_;
	bool null_child_;
	double rad_, tilt_;
	FloatRect vis_;
};
