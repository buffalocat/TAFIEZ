#include "stdafx.h"
#include "modifiertab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"

#include "color_constants.h"

#include "gameobject.h"

#include "wall.h"
#include "car.h"
#include "door.h"
#include "gate.h"
#include "gatebody.h"
#include "pressswitch.h"
#include "autoblock.h"
#include "puppetblock.h"
#include "clearflag.h"
#include "permanentswitch.h"
#include "floorsign.h"
#include "incinerator.h"
#include "flaggate.h"
#include "flagswitch.h"
#include "mapdisplay.h"
#include "autosavepanel.h"

#include "colorcycle.h"

ModifierTab::ModifierTab(EditorState* editor) : EditorTab(editor) {}

ModifierTab::~ModifierTab() {}

// Current object type
static ModCode mod_code = ModCode::Car;

// Model objects that new objects are created from
static Car model_car{ nullptr, CarType::Normal, {} };
static Door model_door{ nullptr, 0, false, true, false, 0 };
static Gate model_gate{ nullptr, nullptr, GREEN, 0, false, false, false, false };
static PressSwitch model_press_switch{ nullptr, GREEN, false, false };
static ClearFlag model_clear_flag{ nullptr, true, false, false };
static PermanentSwitch model_perm_switch{ nullptr, GREEN, false, 0 };
static FloorSign model_floor_sign{ nullptr, "", false };
static Incinerator model_incinerator{ nullptr, 0, false, true, false };
static FlagGate model_flag_gate{ nullptr, 1, 0, 0, false, false, false };
static FlagSwitch model_flag_switch{ nullptr, false, 0 };
static AutosavePanel model_autosave_panel{ nullptr, "", false };

static ColorCycle model_color_cycle{};

// Object Inspection
static GameObject* selected_obj = nullptr;
static Point3 selected_pos = { -1, -1, -1 };

void ModifierTab::init() {
	selected_obj = nullptr;
}

void ModifierTab::main_loop(EditorRoom* eroom) {
	ImGui::Text("The Modifier Tab");
	ImGui::Separator();
	if (!eroom) {
		ImGui::Text("No room loaded.");
		return;
	}

	ImGui::Checkbox("Inspect Mode##MOD_inspect", &inspect_mode_);

	mod_tab_options(eroom->map());
}

void ModifierTab::mod_tab_options(RoomMap* room_map) {
	ObjectModifier* mod = nullptr;
	if (inspect_mode_) {
		if (selected_obj) {
			mod = selected_obj->modifier();
			ImGui::Text("Current selected object position: (%d,%d,%d)", selected_pos.x, selected_pos.y, selected_pos.z);
			if (!mod) {
				ImGui::Text("It currently has no modifier.");
				return;
			}
		} else {
			ImGui::Text("No object selected!");
			return;
		}
	} else {
		ImGui::RadioButton("Door##MOD_object", &mod_code, ModCode::Door);
		ImGui::RadioButton("Car##MOD_object", &mod_code, ModCode::Car);
		ImGui::RadioButton("PressSwitch##MOD_object", &mod_code, ModCode::PressSwitch);
		ImGui::RadioButton("Gate##MOD_object", &mod_code, ModCode::Gate);
		ImGui::RadioButton("AutoBlock##MOD_object", &mod_code, ModCode::AutoBlock);
		ImGui::RadioButton("PuppetBlock##MOD_object", &mod_code, ModCode::PuppetBlock);
		ImGui::RadioButton("ClearFlag##MOD_object", &mod_code, ModCode::ClearFlag);
		ImGui::RadioButton("PermanentSwitch##MOD_object", &mod_code, ModCode::PermanentSwitch);
		ImGui::RadioButton("FloorSign##MOD_object", &mod_code, ModCode::FloorSign);
		ImGui::RadioButton("Incinerator##MOD_object", &mod_code, ModCode::Incinerator);
		ImGui::RadioButton("Flag Gate##MOD_object", &mod_code, ModCode::FlagGate);
		ImGui::RadioButton("Flag Switch##MOD_object", &mod_code, ModCode::FlagSwitch);
		ImGui::RadioButton("Map Display##MOD_object", &mod_code, ModCode::MapDisplay);
		ImGui::RadioButton("Autosave Panel##MOD_object", &mod_code, ModCode::AutosavePanel);
	}
	ImGui::Separator();
	switch (mod ? mod->mod_code() : mod_code) {
	case ModCode::Car:
	{
		ImGui::Text("Car");
		Car* car = mod ? static_cast<Car*>(mod) : &model_car;
		ColorCycle* color_cycle = car ? &car->color_cycle_ : &model_color_cycle;
		ImGui::RadioButton("Normal Car##MOD_CAR_type", &car->type_, CarType::Normal);
		ImGui::RadioButton("Locked Car##MOD_CAR_type", &car->type_, CarType::Locked);
		ImGui::RadioButton("Convertible##MOD_CAR_type", &car->type_, CarType::Convertible);
		ImGui::RadioButton("Hover##MOD_CAR_type", &car->type_, CarType::Hover);
		ImGui::RadioButton("Binding##MOD_CAR_type", &car->type_, CarType::Binding);
		select_color_cycle(color_cycle);
		break;
	}
	case ModCode::Door:
	{
		ImGui::Text("Door");
		Door* door = mod ? static_cast<Door*>(mod) : &model_door;
		ImGui::Checkbox("Persistent?##MOD_DOOR_persistent", &door->persistent_);
		ImGui::Checkbox("Active by Default?##MOD_DOOR_default", &door->default_);
		if (mod) {
			int s_door_id = door->door_id_;
			ImGui::InputInt("Door ID##MOD_DOOR_id", &s_door_id);
			if (s_door_id < 0) {
				s_door_id = 0;
			}
			unsigned int new_door_id = (unsigned int)s_door_id;
			room_map->remove_door(door);
			if (ImGui::Button("Find Unused ID##MOD_DOOR_id_button")) {
				new_door_id = room_map->get_unused_door_id();
			}
			door->door_id_ = new_door_id;
			room_map->add_door(door);
		}
		ImGui::InputScalarN("Flag on Entry (0 = None)##MOD_DOOR_flag", ImGuiDataType_U32, &door->map_flag_, 1);
		break;
	}
	case ModCode::Gate:
	{
		ImGui::Text("Gate");
		Gate* gate = mod ? static_cast<Gate*>(mod) : &model_gate;
		ImGui::Checkbox("Persistent?##MOD_GATE_persistent", &gate->persistent_);
		ImGui::Checkbox("Active by Default?##GATE_default", &gate->default_);
		gate->active_ = gate->default_;
		gate->waiting_ = gate->default_;
		ImGui::InputInt("color##MOD_PRESS_SWITCH_modify_COLOR", &gate->color_);
		color_button(gate->color_);
		break;
	}
	case ModCode::PressSwitch:
	{
		ImGui::Text("PressSwitch");
		PressSwitch* ps = mod ? static_cast<PressSwitch*>(mod) : &model_press_switch;
		ImGui::Checkbox("Persistent?##MOD_PRESS_SWITCH_persistent", &ps->persistent_);
		ImGui::InputInt("color##MOD_PRESS_SWITCH_modify_COLOR", &ps->color_);
		color_button(ps->color_);
		break;
	}
	case ModCode::PermanentSwitch:
	{
		ImGui::Text("PermanentSwitch");
		PermanentSwitch* ps = mod ? static_cast<PermanentSwitch*>(mod) : &model_perm_switch;
		ImGui::InputInt("color##MOD_PERMANENT_SWITCH_modify_COLOR", &ps->color_);
		color_button(ps->color_);
		break;
	}
	case ModCode::ClearFlag:
	{
		ImGui::Text("ClearFlag");
		ClearFlag* cf = mod ? static_cast<ClearFlag*>(mod) : &model_clear_flag;
		ImGui::Checkbox("Real?##MOD_CLEAR_FLAG_real", &cf->real_);
		break;
	}
	case ModCode::FloorSign:
	{
		ImGui::Text("FloorSign");
		FloorSign* sign = mod ? static_cast<FloorSign*>(mod) : &model_floor_sign;
		static char buf[256];
		snprintf(buf, 256, sign->content_.c_str());
		ImGui::InputTextMultiline("Sign Text:##MOD_FLOOR_SIGN_text", buf, 256);
		sign->content_ = std::string(buf);
		break;
	}
	case ModCode::Incinerator:
	{
		ImGui::Text("Incinerator");
		Incinerator* incinerator = mod ? static_cast<Incinerator*>(mod) : &model_incinerator;
		ImGui::Checkbox("Persistent?##MOD_INCINERATOR_persistent", &incinerator->persistent_);
		ImGui::Checkbox("Active by Default?##MOD_INCINERATOR_default", &incinerator->default_);
		break;
	}
	case ModCode::FlagGate:
	{
		ImGui::Text("Flag Gate");
		FlagGate* flag_gate = mod ? static_cast<FlagGate*>(mod) : &model_flag_gate;
		ImGui::InputInt("Num Flags##MOD_FLAGGATE_num_flags", &flag_gate->num_flags_);
		ImGui::InputInt("Orientation##MOD_FLAGGATE_orientation", &flag_gate->orientation_);
		break;
	}
	case ModCode::FlagSwitch:
	{
		ImGui::Text("Flag Switch");
		FlagSwitch* flag_switch = mod ? static_cast<FlagSwitch*>(mod) : &model_flag_switch;
		ImGui::InputInt("Orientation##MOD_FLAGSWITCH_orientation", &flag_switch->orientation_);
		break;
	}
	case ModCode::AutosavePanel:
	{
		ImGui::Text("AutosavePanel");
		AutosavePanel* panel = mod ? static_cast<AutosavePanel*>(mod) : &model_autosave_panel;
		static char buf[256];
		snprintf(buf, 256, panel->label_.c_str());
		ImGui::InputTextMultiline("Autosave Label:##MOD_AUTOSAVE_PANEL_text", buf, 256);
		panel->label_ = std::string(buf);
		break;
	}
	// Trivial objects
	case ModCode::AutoBlock:
	case ModCode::PuppetBlock:
	case ModCode::MapDisplay:
		break;
	}
}

const char* color_ordinals[5] = {
	"First Color##COLOR_CYCLE_color",
	"Second Color##COLOR_CYCLE_color",
	"Third Color##COLOR_CYCLE_color",
	"Fourth Color##COLOR_CYCLE_color",
	"Fifth Color##COLOR_CYCLE_color",
};

void ModifierTab::select_color_cycle(ColorCycle* cycle) {
	ImGui::InputInt("Number of colors##COLOR_CYCLE_num", &cycle->size_);
	clamp(&cycle->size_, 0, MAX_COLOR_CYCLE);
	for (int i = 0; i < cycle->size_; ++i) {
		ImGui::InputInt(color_ordinals[i], &cycle->colors_[i]);
		color_button(cycle->colors_[i]);
	}
}

bool ModifierTab::handle_keyboard_input() {
	if (EditorTab::handle_keyboard_input()) {
		return true;
	}
	if (glfwGetKey(editor_->window_, GLFW_KEY_LEFT_SHIFT)) {
		for (int i = 1; i <= 9; ++i) {
			if (glfwGetKey(editor_->window_, GLFW_KEY_0 + i) == GLFW_PRESS) {
				mod_code = static_cast<ModCode>(i);
			}
		}
	}
	return false;
}

void ModifierTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
	RoomMap* map = eroom->map();
	selected_obj = nullptr;
	selected_pos = pos;
	if (!map->valid(pos)) {
		return;
	}
	GameObject* obj = map->view(pos);
	if (!obj) {
		return;
	} else if (inspect_mode_ || obj->modifier()) {
		selected_obj = obj;
		return;
	}
	std::unique_ptr<ObjectModifier> mod{};
	switch (mod_code) {
	case ModCode::Car:
		mod = std::make_unique<Car>(model_car);
		break;
	case ModCode::Door:
	{
		auto door = std::make_unique<Door>(model_door);
		door->door_id_ = map->get_unused_door_id();
		map->add_door(door.get());
		mod = std::move(door);
		break;
	}
	case ModCode::Gate:
	{
		auto gate = std::make_unique<Gate>(model_gate);
		gate->parent_ = obj;
		map->push_to_object_array(std::make_unique<GateBody>(gate.get(), pos + Point3{ 0, 0, 1 }), nullptr);
		mod = std::move(gate);
		break;
	}
	case ModCode::PressSwitch:
		mod = std::make_unique<PressSwitch>(model_press_switch);
		break;
	case ModCode::AutoBlock:
		mod = std::make_unique<AutoBlock>(obj);
		break;
	case ModCode::PuppetBlock:
		mod = std::make_unique<PuppetBlock>(obj);
		break;
	case ModCode::ClearFlag:
		mod = std::make_unique<ClearFlag>(model_clear_flag);
		break;
	case ModCode::PermanentSwitch:
		mod = std::make_unique<PermanentSwitch>(model_perm_switch);
		break;
	case ModCode::FloorSign:
		mod = std::make_unique<FloorSign>(obj, model_floor_sign.content_, false);
		break;
	case ModCode::Incinerator:
		mod = std::make_unique<Incinerator>(model_incinerator);
		break;
	case ModCode::FlagGate:
		mod = std::make_unique<FlagGate>(model_flag_gate);
		break;
	case ModCode::FlagSwitch:
		mod = std::make_unique<FlagSwitch>(model_flag_switch);
		break;
	case ModCode::MapDisplay:
		mod = std::make_unique<MapDisplay>(obj);
		break;
	case ModCode::AutosavePanel:
		mod = std::make_unique<AutosavePanel>(obj, model_autosave_panel.label_, false);
		break;
	}
	if (!mod->valid_parent(obj)) {
		return;
	}
	// A Wall with a modifier can't be a generic wall
	if (obj->id_ == GENERIC_WALL_ID) {
		map->clear(pos);
		auto new_wall = std::make_unique<Wall>(pos);
		obj = new_wall.get();
		map->create_in_map(std::move(new_wall), false, nullptr);
	}
	mod->parent_ = obj;
	mod->setup_on_editor_creation(editor_->global_.get(), eroom->room.get());
	selected_obj = obj;
	obj->set_modifier(std::move(mod));
}

void ModifierTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
	selected_obj = nullptr;
	// Don't allow deletion in inspect mode
	if (inspect_mode_) {
		return;
	}
	RoomMap* map = eroom->map();
	if (GameObject* obj = map->view(pos)) {
		if (ObjectModifier* mod = obj->modifier()) {
			mod->cleanup_on_editor_destruction(editor_->global_.get());
			// Revert a modified wall to a generic wall!
			if (auto* wall = dynamic_cast<Wall*>(obj)) {
				map->take_from_map(obj, true, false, nullptr);
				map->remove_from_object_array(obj);
				map->create_wall(pos);
			} else {
				mod->cleanup_on_take(map, nullptr, true);
				obj->set_modifier({});
			}
		}
	}
}
