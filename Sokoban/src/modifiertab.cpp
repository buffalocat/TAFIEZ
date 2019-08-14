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
#include "worldresetkey.h"

#include "colorcycle.h"

ModifierTab::ModifierTab(EditorState* editor, GraphicsManager* gfx) : EditorTab(editor, gfx) {}

ModifierTab::~ModifierTab() {}

// Current object type
static ModCode mod_code = ModCode::NONE;

// Model objects that new objects are created from
static Car model_car{ nullptr, {} };
static Door model_door{ nullptr, 0, false, true, false };
static Gate model_gate{ nullptr, nullptr, GREEN, 0, false, false, false, false };
static PressSwitch model_press_switch{ nullptr, GREEN, false, false };
static ClearFlag model_clear_flag{ nullptr, 1, true, false, false, '!' };

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

	mod_tab_options();
}

void ModifierTab::mod_tab_options() {
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
		ImGui::RadioButton("Car##MOD_object", &mod_code, ModCode::Car);
		ImGui::RadioButton("Door##MOD_object", &mod_code, ModCode::Door);
		ImGui::RadioButton("Gate##MOD_object", &mod_code, ModCode::Gate);
		ImGui::RadioButton("PressSwitch##MOD_object", &mod_code, ModCode::PressSwitch);
		ImGui::RadioButton("AutoBlock##MOD_object", &mod_code, ModCode::AutoBlock);
		ImGui::RadioButton("PuppetBlock##MOD_object", &mod_code, ModCode::PuppetBlock);
		ImGui::RadioButton("ClearFlag##MOD_object", &mod_code, ModCode::ClearFlag);
		ImGui::RadioButton("WorldResetKey##MOD_object", &mod_code, ModCode::WorldResetKey);
	}
	ImGui::Separator();
	switch (mod ? mod->mod_code() : mod_code) {
	case ModCode::Car:
	{
		ImGui::Text("Car");
		Car* car = mod ? static_cast<Car*>(mod) : &model_car;
		ColorCycle* color_cycle = car ? &car->color_cycle_ : &model_color_cycle;
		select_color_cycle(color_cycle);
		break;
	}
	case ModCode::Door:
	{
		ImGui::Text("Door");
		Door* door = mod ? static_cast<Door*>(mod) : &model_door;
		ImGui::Checkbox("Persistent?##DOOR_persistent", &door->persistent_);
		ImGui::Checkbox("Active by Default?##DOOR_default", &door->default_);
		break;
	}
	case ModCode::Gate:
	{
		ImGui::Text("Gate");
		Gate* gate = mod ? static_cast<Gate*>(mod) : &model_gate;
		ImGui::Checkbox("Persistent?##GATE_persistent", &gate->persistent_);
		ImGui::Checkbox("Active by Default?##GATE_default", &gate->default_);
		gate->active_ = gate->default_;
		gate->waiting_ = gate->default_;
		ImGui::InputInt("color##PRESS_SWITCH_modify_COLOR", &gate->color_);
		color_button(gate->color_);
		break;
	}
	case ModCode::PressSwitch:
	{
		ImGui::Text("PressSwitch");
		PressSwitch* ps = mod ? static_cast<PressSwitch*>(mod) : &model_press_switch;
		ImGui::Checkbox("Persistent?##PRESS_SWITCH_persistent", &ps->persistent_);
		ImGui::InputInt("color##PRESS_SWITCH_modify_COLOR", &ps->color_);
		color_button(ps->color_);
		break;
	}
	case ModCode::ClearFlag:
	{
		ImGui::Text("ClearFlag");
		ClearFlag* cf = mod ? static_cast<ClearFlag*>(mod) : &model_clear_flag;
		ImGui::Checkbox("Real?##CLEAR_FLAG_real", &cf->real_);
		break;
	}
	// Trivial objects
	case ModCode::AutoBlock:
	case ModCode::PuppetBlock:
	case ModCode::WorldResetKey:
		break;
	default:
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
		mod = std::make_unique<Door>(model_door);
		break;
	case ModCode::Gate:
	{
		auto gate = std::make_unique<Gate>(model_gate);
		gate->parent_ = obj;
		auto gate_body = std::make_unique<GateBody>(gate.get(), pos + Point3{ 0, 0, 1 });
		gate->body_ = gate_body.get();
		map->push_to_object_array(std::move(gate_body), nullptr);
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
	case ModCode::WorldResetKey:
		mod = std::make_unique<WorldResetKey>(obj);
		break;
	default:
		return;
	}
	if (!mod->valid_parent(obj)) {
		return;
	}
	// A Wall with a modifier can't be the global wall
	if (obj->id_ == GENERIC_WALL_ID) {
		map->clear(pos);
		auto new_wall = std::make_unique<Wall>(pos);
		obj = new_wall.get();
		map->create_in_map(std::move(new_wall), nullptr);
	}
	mod->parent_ = obj;
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
			// Revert a modified wall to a generic wall!
			if (auto* wall = dynamic_cast<Wall*>(obj)) {
				map->take_from_map(obj, true, nullptr);
				map->remove_from_object_array(obj);
				map->create_wall(pos);
			} else {
				mod->cleanup_on_take(map, true);
				obj->set_modifier({});
			}
		}
	}
}
