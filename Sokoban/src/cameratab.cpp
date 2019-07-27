#include "stdafx.h"
#include "cameratab.h"
#include "editorstate.h"

#include "common_constants.h"
#include "common_enums.h"
#include "camera.h"

CameraTab::CameraTab(EditorState* editor, GraphicsManager* gfx) : EditorTab(editor, gfx),
xa{}, ya{}, xb{}, yb{}, x_{}, y_{}, w_{}, h_{}, priority_{},
rad_{ DEFAULT_CAM_RADIUS }, tilt_{ DEFAULT_CAM_TILT }, rot_{ DEFAULT_CAM_ROTATION },
xpad_{}, ypad_{}, center_{} {}

CameraTab::~CameraTab() {}

static bool inspect_mode = false;
static CameraContext* selected_cam = nullptr;
static CameraCode cam_code = CameraCode::NONE;

static int* x_ptr = nullptr;
static int* y_ptr = nullptr;
static int* w_ptr = nullptr;
static int* h_ptr = nullptr;
static int* priority_ptr = nullptr;
static float* rad_ptr = nullptr;
static float* tilt_ptr = nullptr;
static float* rot_ptr = nullptr;
static int* xpad_ptr = nullptr;
static int* ypad_ptr = nullptr;
static FPoint3* center_ptr = nullptr;

void CameraTab::init() {
	inspect_mode = false;
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

void CameraTab::camera_type_choice(CameraCode* cam_code_ptr) {
	ImGui::RadioButton("Free##OBJECT_object", cam_code_ptr, CameraCode::Free);
	ImGui::RadioButton("Fixed##OBJECT_object", cam_code_ptr, CameraCode::Fixed);
	ImGui::RadioButton("Clamped##OBJECT_object", cam_code_ptr, CameraCode::Clamped);
	ImGui::RadioButton("Null##OBJECT_object", cam_code_ptr, CameraCode::Null);
}

void CameraTab::main_loop(EditorRoom* eroom) {
	ImGui::Text("The Camera Tab");
	ImGui::Separator();
	if (!eroom) {
		ImGui::Text("No room loaded.");
		return;
	}

	center_.z = (float)eroom->cam_pos.z;

	ImGui::Text("Selected Corners: (%d, %d), (%d, %d)", xa, ya, xb, yb);

	ImGui::Checkbox("Inspect Mode##CAMERA_inspect", &inspect_mode);
	ImGui::Separator();

	rad_ptr = nullptr;
	tilt_ptr = nullptr;
	rot_ptr = nullptr;
	xpad_ptr = nullptr;
	ypad_ptr = nullptr;
	center_ptr = nullptr;

	if (inspect_mode) {
		static int current = 0;
		auto& contexts = eroom->room->camera()->loaded_contexts();
		const char* labels[1024];
		int len = get_context_labels(labels, contexts);
		if (ImGui::ListBox("Camera Contexts##CAMERA", &current, labels, len, len)) {
			selected_cam = contexts[current].get();
		}
		if (selected_cam) {
			xa = selected_cam->x_;
			xb = xa + selected_cam->w_ - 1;
			ya = selected_cam->y_;
			yb = ya + selected_cam->h_ - 1;
			x_ptr = &selected_cam->x_;
			y_ptr = &selected_cam->y_;
			w_ptr = &selected_cam->w_;
			h_ptr = &selected_cam->h_;
			priority_ptr = &selected_cam->priority_;

			if (auto* free = dynamic_cast<FreeCameraContext*>(selected_cam)) {
				rad_ptr = &free->rad_;
				tilt_ptr = &free->tilt_;
				rot_ptr = &free->rot_;
			} else if (auto* fixed = dynamic_cast<FixedCameraContext*>(selected_cam)) {
				rad_ptr = &fixed->rad_;
				tilt_ptr = &fixed->tilt_;
				rot_ptr = &fixed->rot_;
				center_ptr = &fixed->center_;
			} else if (auto* clamped = dynamic_cast<ClampedCameraContext*>(selected_cam)) {
				rad_ptr = &clamped->rad_;
				tilt_ptr = &clamped->tilt_;
				xpad_ptr = &clamped->xpad_;
				ypad_ptr = &clamped->ypad_;
			}
		} else {
			ImGui::Text("No CameraContext selected.");
			return;
		}
	} else {
		camera_type_choice(&cam_code);

		x_ptr = &x_;
		y_ptr = &y_;
		w_ptr = &w_;
		h_ptr = &h_;
		priority_ptr = &priority_;

		switch (cam_code) {
		case CameraCode::Free:
			rad_ptr = &rad_;
			tilt_ptr = &tilt_;
			rot_ptr = &rot_;
			break;
		case CameraCode::Clamped:
			rad_ptr = &rad_;
			tilt_ptr = &tilt_;
			rot_ptr = &rot_;
			xpad_ptr = &xpad_;
			ypad_ptr = &ypad_;
			break;
		case CameraCode::Fixed:
			rad_ptr = &rad_;
			tilt_ptr = &tilt_;
			rot_ptr = &rot_;
			center_ptr = &center_;
			break;
		default:
			break;
		}		
	}

	ImGui::Separator();

	const static int MAX_LABEL_LENGTH = 64;
	static char label_buf[MAX_LABEL_LENGTH] = "";

	if (inspect_mode) {
		if (selected_cam) {
			snprintf(label_buf, MAX_LABEL_LENGTH, "%s", selected_cam->label_.c_str());
			if (ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH)) {
				selected_cam->label_ = std::string(label_buf);
			}
		}
	} else {
		ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH);
	}

	ImGui::Text("Rect: (%d, %d, %d, %d)", *x_ptr, *y_ptr, *w_ptr, *h_ptr);
	ImGui::InputInt("Priority##CAMERA_priority", priority_ptr);
	
	if (rad_ptr) {
		ImGui::InputFloat("Radius##CAMERA_radius", rad_ptr);
	}
	if (tilt_ptr) {
		ImGui::InputFloat("Tilt##CAMERA_tilt", tilt_ptr);
	}
	if (rot_ptr) {
		ImGui::InputFloat("Rotation##CAMERA_rot", rot_ptr);
	}
	if (xpad_ptr) {
		ImGui::InputInt("x-pad##CAMERA_xpad", xpad_ptr);
	}
	if (ypad_ptr) {
		ImGui::InputInt("y-pad##CAMERA_ypad", ypad_ptr);
	}
	if (center_ptr) {
		ImGui::InputFloat("Center x##CAMERA_cx", &(center_ptr->x));
		ImGui::InputFloat("Center y##CAMERA_cy", &(center_ptr->y));
		ImGui::InputFloat("Center z##CAMERA_cz", &(center_ptr->z));
	}

	ImGui::Separator();

	if (inspect_mode) {
		if (ImGui::Button("Erase Selected CameraContext##CAMERA")) {
			eroom->room->camera()->remove_context(selected_cam);
			selected_cam = nullptr;
		}
		return;
	}

	ImGui::Text("%s", label_buf);

	if (ImGui::Button("Make CameraContext##CAMERA")) {
		std::string label{ label_buf };
		std::unique_ptr<CameraContext> new_cam{};
		switch (cam_code) {
		case CameraCode::Free:
			new_cam = std::make_unique<FreeCameraContext>(label, x_, y_, w_, h_, priority_, rad_, tilt_, rot_);
			break;
		case CameraCode::Fixed:
			new_cam = std::make_unique<FixedCameraContext>(label, x_, y_, w_, h_, priority_, rad_, tilt_, rot_, center_);
			break;
		case CameraCode::Clamped:
			new_cam = std::make_unique<ClampedCameraContext>(label, x_, y_, w_, h_, priority_, rad_, tilt_, xpad_, ypad_);
			break;
		case CameraCode::Null:
			new_cam = std::make_unique<NullCameraContext>(label, x_, y_, w_, h_, priority_);
			break;
		default:
			break;
		}
		if (new_cam) {
			eroom->room->camera()->push_context(std::move(new_cam));
		}
	}
}

void CameraTab::normalize_a_b() {
	if (xa <= xb) {
		*x_ptr = xa;
		*w_ptr = xb + 1 - xa;
	} else {
		*x_ptr = xb;
		*w_ptr = xa + 1 - xb;
	}
	if (ya <= yb) {
		*y_ptr = ya;
		*h_ptr = yb + 1 - ya;
	} else {
		*y_ptr = yb;
		*h_ptr = ya + 1 - yb;
	}
	FPoint3* temp_center_ptr = &center_;
	if (center_ptr) {
		temp_center_ptr = center_ptr;
	}
	temp_center_ptr->x = *x_ptr + *w_ptr / 2.0f;
	temp_center_ptr->y = *y_ptr + *h_ptr / 2.0f;
}

// Change behavior for modes...?
void CameraTab::handle_left_click(EditorRoom*, Point3 p) {
	xa = p.x;
	ya = p.y;
	normalize_a_b();
}

void CameraTab::handle_right_click(EditorRoom*, Point3 p) {
	xb = p.x;
	yb = p.y;
	normalize_a_b();
}