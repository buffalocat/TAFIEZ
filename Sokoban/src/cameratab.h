#pragma once

#include "editortab.h"
#include "point.h"

class CameraContext;
enum class CameraCode;
class GeneralContextData;

class CameraTab : public EditorTab {
public:
	CameraTab(EditorState*);
	~CameraTab();
	void init();
	void main_loop(EditorRoom*);
	void handle_left_click(EditorRoom*, Point3);
	void handle_right_click(EditorRoom*, Point3);

	int get_context_labels(const char* labels[], std::string labels_str[], std::vector<std::unique_ptr<CameraContext>>& contexts);

	void normalize_rect_a_b(IntRect* rect);

private:
	std::unique_ptr<GeneralContextData> model_general_data{};
	std::unique_ptr<GeneralContextData> selected_general_data{};
	IntRect* rect_ptr;
};
