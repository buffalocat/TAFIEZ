#include "stdafx.h"

#include "cameratab.h"
#include "editorstate.h"

#include "common_constants.h"
#include "common_enums.h"
#include "camera.h"


CameraTab::CameraTab(EditorState* editor) : EditorTab(editor) {
	model_general_data = std::make_unique<GeneralContextData>();
}

CameraTab::~CameraTab() {}

static CameraContext* selected_cam = nullptr;

enum class PaddingMode {
	Uniform,
	XY,
	All,
};

enum class RadiusMode {
	Custom,
	Larger,
	Smaller,
	ClampX,
	ClampY,
	Default,
};

enum class PositionMode {
	Free,
	Clamp,
	Fix,
};

class GeneralContextData {
public:
	GeneralContextData();
	GeneralContextData(GeneralCameraContext* context);
	~GeneralContextData();
	void load_from_context(GeneralCameraContext* context);
	void unpack_flags();
	void pack_flags();
	void write_to_context(GeneralCameraContext* context);

	void editor_options();

private:
	unsigned int flags = 0;
	bool null_area = false, named_area = true, has_null_child = true, free_cam = false, circ_cam = false;
	PaddingMode pad_mode = PaddingMode::Uniform;
	RadiusMode rad_mode = RadiusMode::Default;
	PositionMode x_pos_mode = PositionMode::Free, y_pos_mode = PositionMode::Free;
	bool tilt_custom = false, rot_custom = false, center_custom = false;
	FPoint3 center{ 0,0,0 };

	IntRect rect{ 0,0,0,0 };
	int priority = 10;
	std::string label = "";
	double rad = DEFAULT_CAM_RADIUS;
	double tilt = DEFAULT_CAM_TILT;
	double rot = DEFAULT_CAM_ROTATION;
	Padding pad{ 0,0,0,0 };

	friend class CameraTab;
};

GeneralContextData::GeneralContextData() {}

GeneralContextData::GeneralContextData(GeneralCameraContext* context) {
	load_from_context(context);
}

GeneralContextData::~GeneralContextData() {}

void GeneralContextData::load_from_context(GeneralCameraContext* context) {
	rect = context->rect_;
	priority = context->priority_;
	flags = context->flags_;
	unpack_flags();
	label = context->label_;
	rad = context->rad_;
	tilt = context->tilt_;
	rot = context->rot_;
	pad = context->pad_;
	center = context->center_point_;
}

void GeneralContextData::unpack_flags() {
	null_area = flags & CAM_FLAGS::NULL_AREA;
	named_area = flags & CAM_FLAGS::NAMED_AREA;
	has_null_child = flags & CAM_FLAGS::HAS_NULL_CHILD;
	free_cam = flags & CAM_FLAGS::FREE_CAM;
	circ_cam = flags & CAM_FLAGS::CIRC_CAMERA;

	if (flags & CAM_FLAGS::PAD_UNIFORM) {
		pad_mode = PaddingMode::Uniform;
	} else if (flags & CAM_FLAGS::PAD_XY) {
		pad_mode = PaddingMode::XY;
	} else { // flags & CAM_FLAGS::PAD_ALL
		pad_mode = PaddingMode::All;
	}
	if (flags & CAM_FLAGS::RAD_SMALLER) {
		rad_mode = RadiusMode::Smaller;
	} else if (flags & CAM_FLAGS::RAD_LARGER) {
		rad_mode = RadiusMode::Larger;
	} else if (flags & CAM_FLAGS::RAD_CLAMP_X) {
		rad_mode = RadiusMode::ClampX;
	} else if (flags & CAM_FLAGS::RAD_CLAMP_Y) {
		rad_mode = RadiusMode::ClampY;
	} else if (flags & CAM_FLAGS::RAD_DEFAULT) {
		rad_mode = RadiusMode::Default;
	} else {
		rad_mode = RadiusMode::Custom;
	}
	if (flags & CAM_FLAGS::POS_CLAMP_X) {
		x_pos_mode = PositionMode::Clamp;
	} else if (flags & CAM_FLAGS::POS_FIX_X) {
		x_pos_mode = PositionMode::Fix;
	} else {
		x_pos_mode = PositionMode::Free;
	}
	if (flags & CAM_FLAGS::POS_CLAMP_Y) {
		y_pos_mode = PositionMode::Clamp;
	} else if (flags & CAM_FLAGS::POS_FIX_Y) {
		y_pos_mode = PositionMode::Fix;
	} else {
		y_pos_mode = PositionMode::Free;
	}
	tilt_custom = flags & CAM_FLAGS::TILT_CUSTOM;
	rot_custom = flags & CAM_FLAGS::ROT_CUSTOM;
	center_custom = flags & CAM_FLAGS::CENTER_CUSTOM;
}

void GeneralContextData::pack_flags() {
	flags = (null_area * CAM_FLAGS::NULL_AREA) |
		(named_area * CAM_FLAGS::NAMED_AREA) |
		(has_null_child * CAM_FLAGS::HAS_NULL_CHILD) |
		(free_cam * CAM_FLAGS::FREE_CAM) |
		(circ_cam * CAM_FLAGS::CIRC_CAMERA) |
		(tilt_custom * CAM_FLAGS::TILT_CUSTOM) |
		(rot_custom * CAM_FLAGS::ROT_CUSTOM) |
		(center_custom * CAM_FLAGS::CENTER_CUSTOM);
	switch (pad_mode) {
	case PaddingMode::Uniform:
		flags |= CAM_FLAGS::PAD_UNIFORM;
		break;
	case PaddingMode::XY:
		flags |= CAM_FLAGS::PAD_XY;
		break;
	case PaddingMode::All:
		flags |= CAM_FLAGS::PAD_ALL;
		break;
	}
	switch (rad_mode) {
	case RadiusMode::Smaller:
		flags |= CAM_FLAGS::RAD_SMALLER;
		break;
	case RadiusMode::Larger:
		flags |= CAM_FLAGS::RAD_LARGER;
		break;
	case RadiusMode::ClampX:
		flags |= CAM_FLAGS::RAD_CLAMP_X;
		break;
	case RadiusMode::ClampY:
		flags |= CAM_FLAGS::RAD_CLAMP_Y;
		break;
	case RadiusMode::Default:
		flags |= CAM_FLAGS::RAD_DEFAULT;
		break;
	}
	switch (x_pos_mode) {
	case PositionMode::Clamp:
		flags |= CAM_FLAGS::POS_CLAMP_X;
		break;
	case PositionMode::Fix:
		flags |= CAM_FLAGS::POS_FIX_X;
		break;
	}
	switch (y_pos_mode) {
	case PositionMode::Clamp:
		flags |= CAM_FLAGS::POS_CLAMP_Y;
		break;
	case PositionMode::Fix:
		flags |= CAM_FLAGS::POS_FIX_Y;
		break;
	}
}

void GeneralContextData::write_to_context(GeneralCameraContext* context) {
	context->rect_ = rect;
	context->priority_ = priority;
	pack_flags();
	context->flags_ = flags;
	context->label_ = label;
	context->rad_ = rad;
	context->tilt_ = tilt;
	context->rot_ = rot;
	context->center_point_ = center;
	context->pad_ = pad;
}

void CameraTab::init() {
	inspect_mode_ = false;
	selected_cam = nullptr;
}

int CameraTab::get_context_labels(const char* labels[], std::string labels_str[], std::vector<std::unique_ptr<CameraContext>>& contexts) {
	int i = 0;
	for (auto& c : contexts) {
		labels_str[i] = c->label_;
		if (labels_str[i].empty()) {
			labels[i] = "(UNNAMED)";
		} else {
			labels[i] = labels_str[i].c_str();
		}
		++i;
	}
	return i;
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

	GeneralContextData* current_general_data = nullptr;

	if (inspect_mode_) {
		static int current = 0;
		auto& contexts = eroom->room->camera()->loaded_contexts();
		static const int MAX_CONTEXT_COUNT = 256;
		static std::string labels_str[MAX_CONTEXT_COUNT];
		const char* labels[MAX_CONTEXT_COUNT];
		int len = get_context_labels(labels, labels_str, contexts);
		auto* prev_selected = selected_cam;
		if (ImGui::ListBox("Camera Contexts##CAMERA", &current, labels, len, len)) {
			selected_cam = contexts[current].get();
		}
		if (selected_cam) {
			if (auto* selected_general_cam = dynamic_cast<GeneralCameraContext*>(selected_cam)) {
				if (selected_cam != prev_selected) {
					selected_general_data = std::make_unique<GeneralContextData>(selected_general_cam);
				}
				current_general_data = selected_general_data.get();
			} else {
				ImGui::Text("Can't edit; not a GeneralCameraContext");
				return;
			}
		} else {
			ImGui::Text("No CameraContext selected.");
			return;
		}
	} else {
		selected_cam = nullptr;
		selected_general_data = nullptr;
		current_general_data = model_general_data.get();
	}

	rect_ptr = &current_general_data->rect;

	current_general_data->editor_options();

	ImGui::Separator();

	if (inspect_mode_) {
		if (auto* selected_general_cam = dynamic_cast<GeneralCameraContext*>(selected_cam)) {
			selected_general_data->write_to_context(selected_general_cam);
		}
		if (ImGui::Button("Erase Selected CameraContext##CAMERA")) {
			eroom->room->camera()->remove_context(selected_cam);
			selected_cam = nullptr;
			selected_general_data = nullptr;
			return;
		}
	} else {
		if (ImGui::Button("Make GeneralCameraContext##CAMERA")) {
			auto new_context = std::make_unique<GeneralCameraContext>(IntRect{}, 0, "", 0);
			model_general_data->write_to_context(new_context.get());
			eroom->room->camera()->push_context(std::move(new_context));
			model_general_data = std::make_unique<GeneralContextData>();
			rect_ptr = &model_general_data->rect;
		}	
	}
}

void GeneralContextData::editor_options() {
	if (named_area) {
		ImGui::Text("Level Name (context label)");
	} else {
		ImGui::Text("Camera Context Label");
	}
	const static int MAX_LABEL_LENGTH = 128;
	static char label_buf[MAX_LABEL_LENGTH] = "";
	snprintf(label_buf, MAX_LABEL_LENGTH, "%s", label.c_str());
	if (ImGui::InputTextMultiline("Label##CAMERA", label_buf, MAX_LABEL_LENGTH, ImVec2(-1.0f, ImGui::GetTextLineHeight() * 3))) {
		label = std::string(label_buf);
	}
	
	ImGui::Text("Context Area Corners: (%d, %d), (%d, %d)", rect.xa, rect.ya, rect.xb, rect.yb);
	
	ImGui::InputInt("Priority##CAMERA", &priority);

	ImGui::Separator();

	ImGui::Checkbox("Null Context##CAMERA", &null_area);
	if (null_area) {
		return;
	}

	ImGui::Checkbox("Named Area##CAMERA", &named_area);
	ImGui::Checkbox("Has Null Child##CAMERA", &has_null_child);
	ImGui::Checkbox("Free-cam Area##CAMERA", &free_cam);
	ImGui::Checkbox("Circ-cam Area##CAMERA", &circ_cam);

	if (circ_cam) {
		rot_custom = false;
	}

	ImGui::Separator();
	ImGui::Text("Padding Mode");
	ImGui::RadioButton("Uniform Padding##CAMERA", &pad_mode, PaddingMode::Uniform);
	ImGui::RadioButton("XY Padding##CAMERA", &pad_mode, PaddingMode::XY);
	ImGui::RadioButton("All Different Padding##CAMERA", &pad_mode, PaddingMode::All);
	switch (pad_mode) {
	case PaddingMode::Uniform:
		ImGui::InputInt("Uniform pad##CAMERA", &pad.left);
		pad.right = pad.top = pad.bottom = pad.left;
		break;
	case PaddingMode::XY:
		ImGui::InputInt("X-pad##CAMERA", &pad.left);
		pad.right = pad.left;
		ImGui::InputInt("Y-pad##CAMERA", &pad.top);
		pad.bottom = pad.top;
		break;
	case PaddingMode::All:
		ImGui::InputInt("Left pad##CAMERA", &pad.left);
		ImGui::InputInt("Right pad##CAMERA", &pad.right);
		ImGui::InputInt("Top pad##CAMERA", &pad.top);
		ImGui::InputInt("Bottom pad##CAMERA", &pad.bottom);
		break;
	}

	ImGui::Separator();

	ImGui::Checkbox("Custom Tilt##CAMERA", &tilt_custom);
	if (tilt_custom) {
		ImGui::InputDouble("Tilt##CAMERA", &tilt);
	} else {
		tilt = DEFAULT_CAM_TILT;
	}
	ImGui::Checkbox("Custom Rotation##CAMERA", &rot_custom);
	if (rot_custom) {
		ImGui::InputDouble("Rotation##CAMERA", &rot);
	} else {
		rot = DEFAULT_CAM_ROTATION;
	}
	ImGui::Checkbox("Custom Rotation##CAMERA", &rot_custom);
	if (rot_custom) {
		ImGui::InputDouble("Rotation##CAMERA", &rot);
	} else {
		rot = DEFAULT_CAM_ROTATION;
	}
	ImGui::Checkbox("Custom Center##CAMERA", &center_custom);
	if (center_custom) {
		ImGui::InputDouble("Center X##CAMERA", &center.x);
		ImGui::InputDouble("Center Y##CAMERA", &center.y);
		ImGui::InputDouble("Center Z##CAMERA", &center.z);
	}

	FloatRect vis = { rect.xa - pad.left, rect.ya - pad.top, rect.xb + pad.right, rect.yb + pad.bottom };
	ImGui::Separator();
	ImGui::Text("Radius Mode");
	ImGui::RadioButton("Clamp to Shorter Dimension##CAMERA", &rad_mode, RadiusMode::Smaller);
	ImGui::RadioButton("Clamp to Longer Dimension (Fixed)##CAMERA", &rad_mode, RadiusMode::Larger);
	ImGui::RadioButton("Clamp to X (moves along Y)##CAMERA", &rad_mode, RadiusMode::ClampX);
	ImGui::RadioButton("Clamp to Y (moves along X)##CAMERA", &rad_mode, RadiusMode::ClampY);
	ImGui::RadioButton("Default Radius Size##CAMERA", &rad_mode, RadiusMode::Default);
	ImGui::RadioButton("Custom Radius Size##CAMERA", &rad_mode, RadiusMode::Custom);
	switch (rad_mode) {
	case RadiusMode::Smaller:
	case RadiusMode::Larger:
		x_pos_mode = PositionMode::Clamp;
		y_pos_mode = PositionMode::Clamp;
		break;
	case RadiusMode::ClampX:
		x_pos_mode = PositionMode::Fix;
		y_pos_mode = PositionMode::Clamp;
		break;
	case RadiusMode::ClampY:
		x_pos_mode = PositionMode::Clamp;
		y_pos_mode = PositionMode::Fix;
		break;
	case RadiusMode::Custom:
		x_pos_mode = PositionMode::Free;
		y_pos_mode = PositionMode::Free;
		ImGui::InputDouble("Radius##CAMERA", &rad);
		break;
	case RadiusMode::Default:
		x_pos_mode = PositionMode::Free;
		y_pos_mode = PositionMode::Free;
		rad = DEFAULT_CAM_RADIUS;
		break;
	}

	// Maybe put manual PositionMode choices here?
}

void CameraTab::normalize_rect_a_b(IntRect* rect) {
	if (rect->xa > rect->xb) {
		std::swap(rect->xa, rect->xb);
	}
	if (rect->ya > rect->yb) {
		std::swap(rect->ya, rect->yb);
	}
}

void CameraTab::handle_left_click(EditorRoom*, Point3 p) {
	if (rect_ptr) {
		rect_ptr->xa = p.x;
		rect_ptr->ya = p.y;
		normalize_rect_a_b(rect_ptr);
	}
}

void CameraTab::handle_right_click(EditorRoom*, Point3 p) {
	if (rect_ptr) {
		rect_ptr->xb = p.x;
		rect_ptr->yb = p.y;
		normalize_rect_a_b(rect_ptr);
	}
}