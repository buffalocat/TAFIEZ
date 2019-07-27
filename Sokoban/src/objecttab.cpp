#include "stdafx.h"
#include "objecttab.h"

#include "color_constants.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"

#include "gameobject.h"
#include "pushblock.h"
#include "snakeblock.h"
#include "wall.h"
#include "gatebody.h"
#include "gate.h"

ObjectTab::ObjectTab(EditorState* editor, GraphicsManager* gfx) :
	EditorTab(editor, gfx),
	room_map_{} {}

ObjectTab::~ObjectTab() {}

// Current object type
static ObjCode obj_code = ObjCode::NONE;
static bool inspect_mode = false;

// Model objects that new objects are created from
static PushBlock model_pb {Point3{0,0,0}, GREEN, true, true, Sticky::None};
static SnakeBlock model_sb {Point3{0,0,0}, PURPLE, true, true, 2};

// Object Inspection
static GameObject* selected_obj = nullptr;
static Point3 selected_pos = { -1,-1,-1 };

void ObjectTab::init() {
    selected_obj = nullptr;
}

void ObjectTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Object Tab");
    ImGui::Separator();

    if (!eroom) {
        ImGui::Text("No room loaded.");
        return;
    }
	room_map_ = eroom->map();

    ImGui::Checkbox("Inspect Mode##OBJECT_inspect", &inspect_mode);
    ImGui::Separator();

    object_tab_options();
}

void ObjectTab::object_type_choice(ObjCode* obj_code_ptr) {
	ImGui::RadioButton("Wall##OBJECT_object", obj_code_ptr, ObjCode::Wall);
	ImGui::RadioButton("PushBlock##OBJECT_object", obj_code_ptr, ObjCode::PushBlock);
	ImGui::RadioButton("SnakeBlock##OBJECT_object", obj_code_ptr, ObjCode::SnakeBlock);
}

void ObjectTab::object_tab_options() {
    GameObject* obj = nullptr;
    if (inspect_mode) {
        if (selected_obj) {
			obj = selected_obj;
			ImGui::Text("Current selected object position: (%d,%d,%d)", selected_pos.x, selected_pos.y, selected_pos.z);
        } else {
            ImGui::Text("No object selected!");
            return;
        }
    } else {
		object_type_choice(&obj_code);
    }
    ImGui::Separator();
    switch (obj ? obj->obj_code() : obj_code) {
    case ObjCode::PushBlock: {
            ImGui::Text("PushBlock");
            PushBlock* pb = obj ? static_cast<PushBlock*>(obj) : &model_pb;
            ImGui::Checkbox("Pushable?##PB_modify_push", &pb->pushable_);
            ImGui::Checkbox("Gravitable?##PB_modify_grav", &pb->gravitable_);
            ImGui::InputInt("color##PB_modify_COLOR", &pb->color_);
			color_button(pb->color_);
            ImGui::Text("Stickiness");
            ImGui::RadioButton("NonStick##PB_modify_sticky", &pb->sticky_, Sticky::None);
            ImGui::RadioButton("Weakly Sticky##PB_modify_sticky", &pb->sticky_, Sticky::Weak);
            ImGui::RadioButton("Strongly Sticky##PB_modify_sticky", &pb->sticky_, Sticky::Strong);
            ImGui::RadioButton("Weak+Strong Sticky##PB_modify_sticky", &pb->sticky_, Sticky::AllStick);
        }
        break;
    case ObjCode::SnakeBlock: {
            ImGui::Text("SnakeBlock");
            SnakeBlock* sb = obj ? static_cast<SnakeBlock*>(obj) : &model_sb;
            ImGui::Checkbox("Pushable?##SB_modify_push", &sb->pushable_);
            ImGui::Checkbox("Gravitable?##SB_modify_grav", &sb->gravitable_);
            ImGui::InputInt("color##SB_modify_COLOR", &sb->color_);
			color_button(sb->color_);
            ImGui::Text("Number of Ends");
            ImGui::RadioButton("One Ended##SB_modify_snake_ends", &sb->ends_, 1);
            ImGui::RadioButton("Two Ended##SB_modify_snake_ends", &sb->ends_, 2);
        }
        break;
    case ObjCode::GateBody: {
            ImGui::Text("GateBody");
            Point3 p_pos = static_cast<GateBody*>(obj)->gate_pos();
            ImGui::Text("See parent Gate at (%d,%d,%d)", p_pos.x, p_pos.y, p_pos.z);
        }
    case ObjCode::Wall: // No parameters for Wall
    default:
        break;
    }
	if (inspect_mode && selected_obj) {
		static ObjCode transmute_obj_code;
		ImGui::Separator();
		ImGui::Text("Transmute Object");
		object_type_choice(&transmute_obj_code);
		if (ImGui::Button("Transmute##OBJECT")) {
			if (auto new_obj = create_from_model(transmute_obj_code, selected_obj)) {
				if (selected_obj->id_ == GLOBAL_WALL_ID) {
					room_map_->clear(selected_pos);
				}
				else {
					room_map_->destroy(selected_obj, nullptr);
				}
				selected_obj = new_obj.get();
				room_map_->create(std::move(new_obj), nullptr);
			}
			// If the new object was a generic Wall, we don't have a new pointer
			else if (transmute_obj_code == ObjCode::Wall) {
				// If the old object was also a generic Wall, do nothing
				if (selected_obj->id_ != GLOBAL_WALL_ID) {
					room_map_->destroy(selected_obj, nullptr);
					room_map_->create_wall(selected_pos);
					selected_obj = room_map_->view(selected_pos);
				}
			}
		}
	}
}

std::unique_ptr<GameObject> ObjectTab::create_from_model(ObjCode obj_code, GameObject* prev) {
	std::unique_ptr<GameObject> obj{};
	switch (obj_code) {
	case ObjCode::Wall:
		if (!(prev && prev->modifier_)) {
			return nullptr;
		}
		obj = std::make_unique<Wall>(selected_pos);
		break;
	case ObjCode::PushBlock:
		obj = std::make_unique<PushBlock>(model_pb);
		break;
	case ObjCode::SnakeBlock:
		obj = std::make_unique<SnakeBlock>(model_sb);
		break;
	default:
		return nullptr;
	}
	if (prev) {
		if (auto* mod = prev->modifier()) {
			if (!mod->valid_parent(obj.get())) {
				return nullptr;
			}
		}
		obj->modifier_ = std::move(prev->modifier_);
		prev->modifier_.reset(nullptr);
	}
	obj->pos_ = selected_pos;
	return std::move(obj);
}

void ObjectTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    if (!room_map_->valid(pos)) {
        selected_obj = nullptr;
        return;
    // Even in create mode, let the user select an already-created object
    }
	selected_pos = pos;
	if (inspect_mode || room_map_->view(pos)) {
        selected_obj = room_map_->view(pos);
        return;
    }
	if (std::unique_ptr<GameObject> obj = create_from_model(obj_code, nullptr)) {
		selected_obj = obj.get();
		room_map_->create(std::move(obj), nullptr);
	}
	else if (obj_code == ObjCode::Wall) {
		room_map_->create_wall(pos);
		selected_obj = room_map_->view(pos);
	}
}

void ObjectTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    // Don't allow deletion in inspect mode
    if (inspect_mode) {
        return;
    }
    GameObject* obj = room_map_->view(pos);
    if (obj) {
        if (obj->obj_code() == ObjCode::Player) {
            return;
        }
        selected_obj = nullptr;
        // When we "destroy" a wall, it doesn't actually destroy the unique Wall object
		if (obj->id_ == GLOBAL_WALL_ID) {
			room_map_->at(pos) = 0;
		}
		else {
			room_map_->destroy(obj, nullptr);
		}
    }
}
