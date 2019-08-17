#include "stdafx.h"
#include "editortab.h"

#include "point.h"
#include "editorstate.h"
#include "color_constants.h"

EditorTab::EditorTab(EditorState* editor) : editor_{ editor }, gfx_{ editor->gfx_ } {}

EditorTab::~EditorTab() {}

void EditorTab::init() {}

bool EditorTab::handle_keyboard_input() {
	if (glfwGetKey(editor_->window_, GLFW_KEY_R)) {
		inspect_mode_ = !inspect_mode_;
		return true;
	}
	return false;
}

void EditorTab::handle_left_click(EditorRoom* eroom, Point3 pos) {}

void EditorTab::handle_right_click(EditorRoom* eroom, Point3 pos) {}

void color_button(int color_id) {
	glm::vec4 color = COLOR_VECTORS[color_id];
	ImGui::ColorButton("##COLOR_BUTTON", ImVec4(color.x, color.y, color.z, color.w), 0, ImVec2(40, 40));
}

void file_choice(std::filesystem::path path, char* output, int max_size) {
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.is_regular_file()) {
			std::string file_name = entry.path().stem().string();
			if (ImGui::Selectable(file_name.c_str())) {
				if (snprintf(output, max_size, file_name.c_str()) >= max_size) {
					std::cout << "The file name '" << output << "' is too large" << std::endl;
				}
				break;
			}
		}
	}
}
