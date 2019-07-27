#include "stdafx.h"
#include "cameratab.h"
#include "editorstate.h"

#include "common_enums.h"
#include "camera.h"

CameraTab::CameraTab(EditorState* editor, GraphicsManager* gfx) : EditorTab(editor, gfx),
x_{}, y_{}, w_{}, h_{}, priority_{}, rad_{}, tilt_{}, rot_{}, xpad_{}, ypad_{}, center_{} {}

CameraTab::~CameraTab() {}

static bool inspect_mode = false;
static CameraContext* selected_cam = nullptr;
static CamCode cam_code = CamCode::NONE;

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
static Point3* center_ptr = nullptr;

void CameraTab::init() {
	//RESET VARIABLES HERE
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

void CameraTab::camera_type_choice(CamCode* cam_code_ptr) {
	ImGui::RadioButton("Free##OBJECT_object", cam_code_ptr, CamCode::Free);
	ImGui::RadioButton("Fixed##OBJECT_object", cam_code_ptr, CamCode::Fixed);
	ImGui::RadioButton("Clamped##OBJECT_object", cam_code_ptr, CamCode::Clamped);
	ImGui::RadioButton("Null##OBJECT_object", cam_code_ptr, CamCode::Null);
}

void CameraTab::main_loop(EditorRoom* eroom) {
	ImGui::Text("The Camera Tab");
	ImGui::Separator();
	if (!eroom) {
		ImGui::Text("No room loaded.");
		return;
	}
	ImGui::Checkbox("Inspect Mode##CAMERA_inspect", &inspect_mode);
	ImGui::Separator();

	if (inspect_mode) {
		static int current = 0;
		auto& contexts = eroom->room->camera()->loaded_contexts();
		const char* labels[1024];
		int len = get_context_labels(labels, contexts);
		if (ImGui::ListBox("Camera Contexts##CAMERA", &current, labels, len, len)) {
			selected_cam = contexts[current].get();
		}
		if (selected_cam) {
			x_ptr = &selected_cam->x_;
			y_ptr = &selected_cam->y_;
			w_ptr = &selected_cam->w_;
			h_ptr = &selected_cam->h_;
			priority_ptr = &selected_cam->priority_;
			rad_ptr = nullptr;
			tilt_ptr = nullptr;
			rot_ptr = nullptr;
			xpad_ptr = nullptr;
			ypad_ptr = nullptr;
			center_ptr = nullptr;
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
		x_ptr = &x_;
		y_ptr = &y_;
		w_ptr = &w_;
		h_ptr = &h_;
		priority_ptr = &priority_;
		rad_ptr = &rad_;
		tilt_ptr = &tilt_;
		rot_ptr = &rot_;
		xpad_ptr = &xpad_;
		ypad_ptr = &ypad_;
		center_ptr = &center_;

		camera_type_choice(&cam_code);
	}

	/*
	ImGui::Text("Switches");
	for (int i = 0; i < switches->size(); ++i) {
		Switch* s = (*switches)[i];
		Point3 pos = s->pos();
		ImGui::Text("%s at (%d,%d,%d)", s->parent_->to_str().c_str(), pos.x, pos.y, pos.z);
		ImGui::SameLine();
		char buf[32];
		sprintf_s(buf, "Erase##SWITCH_a_%d", i);
		if (ImGui::Button(buf)) {
			erase_switch(s);
			switches->erase(std::remove(switches->begin(), switches->end(), s), switches->end());
		}
	}

	ImGui::Separator();

	ImGui::Text("Switchables");
	for (int i = 0; i < switchables->size(); ++i) {
		Switchable* s = (*switchables)[i];
		Point3 pos = s->pos();
		ImGui::Text("%s at (%d,%d,%d)", s->parent_->to_str().c_str(), pos.x, pos.y, pos.z);
		ImGui::SameLine();
		char buf[32];
		sprintf_s(buf, "Erase##SWITCH_b_%d", i);
		if (ImGui::Button(buf)) {
			erase_switchable(s);
			switchables->erase(std::remove(switchables->begin(), switchables->end(), s), switchables->end());
		}
	}

	ImGui::Separator();

	if (!inspect_mode) {
		if (ImGui::Button("Empty Queued Objects##SWITCH")) {
			model_switches_.clear();
			model_switchables_.clear();
		}
	}

	const static int MAX_LABEL_LENGTH = 64;
	static char label_buf[MAX_LABEL_LENGTH] = "";

	persistent = &model_persistent_;
	threshold = &model_threshold_;
	if (inspect_mode) {
		if (selected_sig) {
			snprintf(label_buf, MAX_LABEL_LENGTH, "%s", selected_sig->label_.c_str());
			if (ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH)) {
				selected_sig->label_ = std::string(label_buf);
			}
			persistent = &selected_sig->persistent_;
			threshold = &selected_sig->threshold_;
		} else {
			return;
		}
	} else {
		ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH);
		persistent = &model_persistent_;
		threshold = &model_threshold_;
	}

	ImGui::Checkbox("Persistent?##SWITCH_persistent", persistent);

	ImGui::Separator();
	ImGui::Text("Activation Threshold:");

	ImGui::RadioButton("All##SWITCH_threshold", &threshold_mode, Threshold::All);
	ImGui::RadioButton("Any##SWITCH_threshold", &threshold_mode, Threshold::Any);
	ImGui::RadioButton("Custom##SWITCH_threshold", &threshold_mode, Threshold::Custom);

	switch (threshold_mode) {
	case Threshold::All:
		*threshold = (int)switches->size();
		break;
	case Threshold::Any:
		*threshold = 1;
		break;
	case Threshold::Custom:
		ImGui::InputInt("Switch Threshold##SWITCH_threshold", threshold);
		if (*threshold < 0) {
			*threshold = 0;
		}
		break;
	}
	*/

	ImGui::Separator();

	if (inspect_mode) {
		if (ImGui::Button("Erase Selected CameraContext##CAMERA")) {
			eroom->room->camera()->remove_context(selected_cam);
			selected_sig = nullptr;
		}
		return;
	}

	if (ImGui::Button("Make CameraContext##CAMERA")) {
		std::string label{ label_buf };
		auto signaler = std::make_unique<Signaler>(label, 0, *threshold, *persistent, false);
		for (auto& obj : model_switches_) {
			signaler->push_switch_mutual(obj);
		}
		for (auto& obj : model_switchables_) {
			signaler->push_switchable_mutual(obj);
		}
		eroom->map()->push_signaler(std::move(signaler));
		model_switches_.clear();
		model_switchables_.clear();
	}
}

void CameraTab::handle_left_click(EditorRoom*, Point3) {
	
}

void CameraTab::handle_right_click(EditorRoom*, Point3) {}