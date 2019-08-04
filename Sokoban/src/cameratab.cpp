#include "stdafx.h"

#include "cameratab.h"
#include "editorstate.h"

#include "common_constants.h"
#include "common_enums.h"
#include "camera.h"

// Half of the vertical FOV angle
static double VERT_ANGLE;
// 2*width_factor*radius = width of visible rectangle
static double WIDTH_FACTOR;

CameraTab::CameraTab(EditorState* editor, GraphicsManager* gfx) : EditorTab(editor, gfx),
label_{}, rect_{}, vis_{}, priority_{10},
rad_{ DEFAULT_CAM_RADIUS }, tilt_{ DEFAULT_CAM_TILT } {
	VERT_ANGLE = FOV_VERTICAL / 2.0;
	WIDTH_FACTOR = tan(VERT_ANGLE) * ASPECT_RATIO;
}

CameraTab::~CameraTab() {}

static CameraContext* selected_cam = nullptr;

static IntRect* rect_ptr = nullptr;
static int* priority_ptr = nullptr;
static bool* named_area_ptr = nullptr;
static bool* null_child_ptr = nullptr;
static double* rad_ptr = nullptr;
static double* tilt_ptr = nullptr;

static bool bind_vis_to_area = false;
static double max_radius = DEFAULT_CAM_RADIUS;

static FloatRect computed_center = {0,0,0,0};

void CameraTab::init() {
	inspect_mode_ = false;
	selected_cam = nullptr;
}

int CameraTab::get_context_labels(const char* labels[], std::vector<std::unique_ptr<CameraContext>>& contexts) {
	int i = 0;
	for (auto& c : contexts) {
		if (c->label_.empty()) {
			labels[i] = "(UNNAMED)";
		} else {
			labels[i] = c->label_.c_str();
		}
		++i;
	}
	return i;
}

FloatRect compute_vis_from_center(FloatRect center) {
	double tilt = *tilt_ptr;
	double hor_excess_factor = (WIDTH_FACTOR * cos(VERT_ANGLE) * cos(tilt)) / cos(VERT_ANGLE - tilt);
	double upper_excess_factor = tan(VERT_ANGLE + tilt) * cos(tilt) - sin(tilt);
	double lower_excess_factor = tan(VERT_ANGLE - tilt) * cos(tilt) + sin(tilt);
	double rad = *rad_ptr;
	return FloatRect{ round(center.xa - rad * hor_excess_factor + 0.5), center.ya - rad * upper_excess_factor + 0.5,
		center.xb + rad * hor_excess_factor - 0.5, center.yb + rad * lower_excess_factor - 0.5};
}

double compute_actual_radius(FloatRect vis) {
	double tilt = *tilt_ptr;
	double hor_excess_factor = (WIDTH_FACTOR * cos(VERT_ANGLE) * cos(tilt)) / cos(VERT_ANGLE - tilt);
	double hor_max_radius = (vis.xb + 1 - vis.xa) / (2 * hor_excess_factor);
	double upper_excess_factor = tan(VERT_ANGLE + tilt) * cos(tilt) - sin(tilt);
	double lower_excess_factor = tan(VERT_ANGLE - tilt) * cos(tilt) + sin(tilt);
	double vert_max_radius = (vis.yb + 1 - vis.ya) / (upper_excess_factor + lower_excess_factor);
	return std::min(max_radius, std::min(hor_max_radius, vert_max_radius));
}

FloatRect compute_center_from_vis(FloatRect vis) {
	double tilt = *tilt_ptr;
	double hor_excess_factor = (WIDTH_FACTOR * cos(VERT_ANGLE) * cos(tilt)) / cos(VERT_ANGLE - tilt);
	double upper_excess_factor = tan(VERT_ANGLE + tilt) * cos(tilt) - sin(tilt);
	double lower_excess_factor = tan(VERT_ANGLE - tilt) * cos(tilt) + sin(tilt);
	double rad = *rad_ptr;
	auto f = FloatRect{ vis.xa + rad * hor_excess_factor - 0.5, vis.ya + rad * upper_excess_factor - 0.5,
		vis.xb - rad * hor_excess_factor + 0.5, vis.yb - rad * lower_excess_factor + 0.5 };
	return f;
}

void CameraTab::main_loop(EditorRoom* eroom) {
	ImGui::Text("The Camera Tab");
	ImGui::Separator();
	if (!eroom) {
		ImGui::Text("No room loaded.");
		return;
	}

	ImGui::Checkbox("Inspect Mode##CAMERA_inspect", &inspect_mode_);
	ImGui::Separator();

	rad_ptr = &rad_;
	tilt_ptr = &tilt_;

	if (inspect_mode_) {
		static int current = 0;
		auto& contexts = eroom->room->camera()->loaded_contexts();
		const char* labels[1024];
		int len = get_context_labels(labels, contexts);
		auto* prev_selected = selected_cam;
		if (ImGui::ListBox("Camera Contexts##CAMERA", &current, labels, len, len)) {
			selected_cam = contexts[current].get();
		}
		if (selected_cam) {
			rect_ptr = &selected_cam->rect_;
			priority_ptr = &selected_cam->priority_;
			named_area_ptr = &selected_cam->named_area_;
			null_child_ptr = &selected_cam->null_child_;
			if (auto* clamped = dynamic_cast<ClampedCameraContext*>(selected_cam)) {
				rad_ptr = &clamped->rad_;
				tilt_ptr = &clamped->tilt_;
				if (selected_cam != prev_selected) {
					max_radius = clamped->rad_;
					vis_ = compute_vis_from_center(clamped->center_);
				}
			}
		} else {
			ImGui::Text("No CameraContext selected.");
			return;
		}
	} else {
		rect_ptr = &rect_;
		priority_ptr = &priority_;
		named_area_ptr = &named_area_;
		null_child_ptr = &null_child_;
		rad_ptr = &rad_;
		tilt_ptr = &tilt_;
	}

	ImGui::Separator();

	const static int MAX_LABEL_LENGTH = 64;
	static char label_buf[MAX_LABEL_LENGTH] = "";

	if (inspect_mode_) {
		if (selected_cam) {
			snprintf(label_buf, MAX_LABEL_LENGTH, "%s", selected_cam->label_.c_str());
			if (ImGui::InputText("Label##CAMERA", label_buf, MAX_LABEL_LENGTH)) {
				selected_cam->label_ = std::string(label_buf);
			}
			if (ImGui::Button("Erase Selected CameraContext##CAMERA")) {
				eroom->room->camera()->remove_context(selected_cam);
				selected_cam = nullptr;
				return;
			}
		}
	} else {
		ImGui::InputText("Label##CAMERA", label_buf, MAX_LABEL_LENGTH);
	}

	label_ = label_buf;

	ImGui::Text("Context Area Corners: (%d, %d), (%d, %d)", rect_ptr->xa, rect_ptr->ya, rect_ptr->xb, rect_ptr->yb);
	ImGui::Checkbox("Match Visible to Context Area?", &bind_vis_to_area);
	if (bind_vis_to_area) {
		vis_ = { rect_ptr->xa, rect_ptr->ya, rect_ptr->xb, rect_ptr->yb };
	} else {
		ImGui::Text("Visible Corners: (%.2f, %.2f), (%.2f, %.2f)", vis_.xa, vis_.ya, vis_.xb, vis_.yb);
	}

	ImGui::InputInt("Priority##CAMERA", priority_ptr);
	
	ImGui::Separator();

	ImGui::InputDouble("Max Radius##CAMERA", &max_radius);
	if (tilt_ptr) {
		ImGui::InputDouble("Tilt##CAMERA", tilt_ptr);
	}
	*rad_ptr = compute_actual_radius(vis_);
	ImGui::Text("Actual Radius: %f", *rad_ptr);
	ImGui::Checkbox("Named Area?##CAMERA", named_area_ptr);
	ImGui::Checkbox("Null Boundary?##CAMERA", null_child_ptr);

	if (inspect_mode_) {
		if (auto* clamped = dynamic_cast<ClampedCameraContext*>(selected_cam)) {
			clamped->center_ = compute_center_from_vis(vis_);
		}
	} else {
		if (ImGui::Button("Make CameraContext##CAMERA")) {
			compute_center_from_vis(vis_);
			eroom->room->camera()->push_context(std::make_unique<ClampedCameraContext>(label_, rect_, priority_, named_area_, null_child_, rad_, tilt_, compute_center_from_vis(vis_)));
		}	
	}
}

void CameraTab::normalize_rect_a_b() {
	if (rect_ptr->xa > rect_ptr->xb) {
		int temp = rect_ptr->xa;
		rect_ptr->xa = rect_ptr->xb;
		rect_ptr->xb = temp;
	}
	if (rect_ptr->ya > rect_ptr->yb) {
		int temp = rect_ptr->ya;
		rect_ptr->ya = rect_ptr->yb;
		rect_ptr->yb = temp;
	}
}

void CameraTab::normalize_vis_a_b() {
	if (vis_.xa > vis_.xb) {
		double temp = vis_.xa;
		vis_.xa = vis_.xb;
		vis_.xb = temp;
	}
	if (vis_.ya > vis_.yb) {
		double temp = vis_.ya;
		vis_.ya = vis_.yb;
		vis_.yb = temp;
	}
}

void CameraTab::handle_left_click(EditorRoom*, Point3 p) {
	if (glfwGetKey(editor_->window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		if (!bind_vis_to_area) {
			vis_.xa = (double)p.x;
			vis_.ya = (double)p.y;
			normalize_vis_a_b();
		}
	} else {
		rect_ptr->xa = p.x;
		rect_ptr->ya = p.y;
		normalize_rect_a_b();
	}
}

void CameraTab::handle_right_click(EditorRoom*, Point3 p) {
	if (glfwGetKey(editor_->window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		if (!bind_vis_to_area) {
			vis_.xb = (double)p.x;
			vis_.yb = (double)p.y;
			normalize_vis_a_b();
		}
	} else {
		rect_ptr->xb = p.x;
		rect_ptr->yb = p.y;
		normalize_rect_a_b();
	}
}