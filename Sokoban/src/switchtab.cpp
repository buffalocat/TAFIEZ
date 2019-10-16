#include "stdafx.h"
#include "switchtab.h"

#include "gameobject.h"
#include "editorstate.h"
#include "room.h"
#include "roommap.h"
#include "switch.h"
#include "signaler.h"
#include "switchable.h"

SwitchTab::SwitchTab(EditorState* editor) : EditorTab(editor),
model_switches_{}, model_switchables_{}, model_p_switchables_{},
switches_ptr_{}, switchables_ptr_{}, p_switchables_ptr_{},
model_label_{}, model_threshold_{ 1 }, model_parity_level_{ 2 },
parity_index_{ 0 },
sig_type_{ SignalerType::Threshold } {}

SwitchTab::~SwitchTab() {}

static Signaler* selected_sig = nullptr;

void SwitchTab::init() {
	model_switches_.clear();
	model_switchables_.clear();
	for (auto& swlist : model_p_switchables_) {
		swlist.clear();
	}
	model_label_ = {};
	model_threshold_ = 1;
	model_parity_level_ = 2;
	selected_sig = nullptr;
}

int SwitchTab::get_signaler_labels(const char* labels[], std::string labels_str[], std::vector<std::unique_ptr<Signaler>>& signalers) {
	int i = 0;
	for (auto& s : signalers) {
		labels_str[i] = s->label_;
		if (labels_str[i].empty()) {
			labels[i] = "(UNNAMED)";
		} else {
			labels[i] = labels_str[i].c_str();
		}
		++i;
	}
	return i;
}

void erase_switch(Switch* sw, std::vector<Switch*>* switch_group) {
	switch_group->erase(std::remove(switch_group->begin(), switch_group->end(), sw), switch_group->end());
	sw->remove_signaler(selected_sig);
}

void erase_switchable(Switchable* swble, std::vector<Switchable*>* switchable_group) {
	switchable_group->erase(std::remove(switchable_group->begin(), switchable_group->end(), swble), switchable_group->end());
	swble->remove_signaler(selected_sig);
}


void SwitchTab::main_loop(EditorRoom* eroom) {
	ImGui::Text("The Switch Tab");
	ImGui::Separator();
	if (!eroom) {
		ImGui::Text("No room loaded.");
		return;
	}
	ImGui::Checkbox("Inspect Mode##SWITCH_inspect", &inspect_mode_);
	ImGui::Separator();

	if (inspect_mode_) {
		static int current = 0;
		auto& signalers = eroom->map()->signalers_;
		static const int MAX_SIG_COUNT = 1024;
		static std::string labels_str[MAX_SIG_COUNT];
		const char* labels[MAX_SIG_COUNT];
		int len = get_signaler_labels(labels, labels_str, signalers);
		if (ImGui::ListBox("Signalers##SWITCH", &current, labels, len, len)) {
			selected_sig = signalers[current].get();
			// We could put a virtual sig_type() method in Signaler, but this would be the only place it got used
			if (dynamic_cast<ThresholdSignaler*>(selected_sig)) {
				sig_type_ = SignalerType::Threshold;
			} else if (dynamic_cast<ParitySignaler*>(selected_sig)) {
				sig_type_ = SignalerType::Parity;
			}
		}
		if (!selected_sig) {
			ImGui::Text("No Signaler selected.");
			return;
		}
	} else {
		selected_sig = nullptr;
		ImGui::RadioButton("Threshold Signaler##SWITCH", &sig_type_, SignalerType::Threshold);
		ImGui::RadioButton("Parity Signaler##SWITCH", &sig_type_, SignalerType::Parity);
	}

	const static int MAX_LABEL_LENGTH = 64;
	static char label_buf[MAX_LABEL_LENGTH] = "";
	if (inspect_mode_) {
		if (selected_sig) {
			snprintf(label_buf, MAX_LABEL_LENGTH, "%s", selected_sig->label_.c_str());
			if (ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH)) {
				selected_sig->label_ = std::string(label_buf);
			}
		}
	} else {
		ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH);
		model_label_ = std::string(label_buf);
	}

	if (inspect_mode_) {
		switches_ptr_ = &selected_sig->switches_;
	} else {
		switches_ptr_ = &model_switches_;
	}
	print_switch_list(switches_ptr_, "Switches");

	switch (sig_type_) {
	case SignalerType::Threshold:
		threshold_signaler_options(dynamic_cast<ThresholdSignaler*>(selected_sig), eroom->map());
		break;
	case SignalerType::Parity:
		parity_signaler_options(dynamic_cast<ParitySignaler*>(selected_sig), eroom->map());
		break;
	default:
		break;
	}
}

void SwitchTab::print_switch_list(std::vector<Switch*>* switch_group, const char* group_label) {
	ImGui::Text(group_label);
	for (int i = 0; i < switch_group->size(); ++i) {
		Switch* s = (*switch_group)[i];
		Point3 pos = s->pos();
		ImGui::Text("%s at (%d,%d,%d)", s->parent_->to_str().c_str(), pos.x, pos.y, pos.z);
		ImGui::SameLine();
		char buf[64];
		sprintf_s(buf, "Erase##SWITCH_%s_%d", group_label, i);
		if (ImGui::Button(buf)) {
			erase_switch(s, switch_group);
		}
	}
}

void SwitchTab::print_switchable_list(std::vector<Switchable*>* switchable_group, const char* group_label) {
	ImGui::Text(group_label);
	for (int i = 0; i < switchable_group->size(); ++i) {
		Switchable* s = (*switchable_group)[i];
		Point3 pos = s->pos();
		ImGui::Text("%s at (%d,%d,%d)", s->parent_->to_str().c_str(), pos.x, pos.y, pos.z);
		ImGui::SameLine();
		char buf[64];
		sprintf_s(buf, "Erase##SWITCH_%s_%d", group_label, i);
		if (ImGui::Button(buf)) {
			erase_switchable(s, switchable_group);
		}
	}
}

enum class Threshold {
	All,
	Any,
	Custom,
};

void SwitchTab::threshold_signaler_options(ThresholdSignaler* t_sig, RoomMap* map) {
	static int* threshold = nullptr;
	static int switch_count = 0;

	if (inspect_mode_) {
		threshold = &t_sig->threshold_;
		switchables_ptr_ = &t_sig->switchables_;
		switch_count = (int)t_sig->switches_.size();
	} else {
		threshold = &model_threshold_;
		switchables_ptr_ = &model_switchables_;
		switch_count = (int)model_switches_.size();
	}
	
	print_switchable_list(switchables_ptr_, "Switchables");

	ImGui::Separator();
	ImGui::Text("Activation Threshold:");

	static Threshold threshold_mode = Threshold::All;
	static ThresholdSignaler* prev_sig = nullptr;
	static bool fresh_sig = false;

	if (prev_sig != t_sig) {
		fresh_sig = true;
		prev_sig = t_sig;
	}

	// Reset the mode whenever we switch signalers
	if (fresh_sig) {
		if (*threshold == switch_count) {
			threshold_mode = Threshold::All;
		} else if (*threshold == 1) {
			threshold_mode = Threshold::Any;
		} else {
			threshold_mode = Threshold::Custom;
		}
	}

	fresh_sig = false;

	ImGui::RadioButton("All##SWITCH_threshold", &threshold_mode, Threshold::All);
	ImGui::RadioButton("Any##SWITCH_threshold", &threshold_mode, Threshold::Any);
	ImGui::RadioButton("Custom##SWITCH_threshold", &threshold_mode, Threshold::Custom);

	switch (threshold_mode) {
	case Threshold::All:
		*threshold = switch_count;
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

	ImGui::Separator();

	if (inspect_mode_) {
		if (ImGui::Button("Erase Selected Signaler##SWITCH")) {
			for (auto* s : t_sig->switches_) {
				s->remove_signaler(t_sig);
			}
			for (auto* s : t_sig->switchables_) {
				s->remove_signaler(t_sig);
			}
			map->remove_signaler(t_sig);
			selected_sig = nullptr;
		}
	} else {
		if (!inspect_mode_) {
			if (ImGui::Button("Empty Queued Objects##SWITCH")) {
				model_switches_.clear();
				model_switchables_.clear();
			}
		}
		if (ImGui::Button("Make Signaler##SWITCH")) {
			auto new_sig = std::make_unique<ThresholdSignaler>(model_label_, 0, model_threshold_);
			for (auto* obj : model_switches_) {
				new_sig->push_switch(obj, true);
			}
			model_switches_.clear();
			for (auto* obj : model_switchables_) {
				new_sig->push_switchable(obj, true, 0);
			}
			model_switchables_.clear();
			map->push_signaler(std::move(new_sig));
		}
	}
}

void SwitchTab::parity_signaler_options(ParitySignaler* p_sig, RoomMap* map) {
	static int* parity_level = nullptr;

	if (inspect_mode_) {
		p_switchables_ptr_ = &p_sig->switchables_;
		parity_level = &p_sig->parity_level_;
	} else {
		p_switchables_ptr_ = &model_p_switchables_;
		parity_level = &model_parity_level_;
	}

	ImGui::InputInt("Parity Level##SWITCH_parity_level", parity_level);
	if (*parity_level < 2) {
		*parity_level = 2;
	}

	// Keep the vector of switchable groups consistent with the parity level
	int groups = (int)p_switchables_ptr_->size();
	int diff = *parity_level - groups;
	for (int i = 0; i < diff; ++i) {
		p_switchables_ptr_->push_back({});
	}
	if (inspect_mode_) {
		for (int i = *parity_level; i < groups; ++i) {
			for (auto* s : (*p_switchables_ptr_)[i]) {
				s->remove_signaler(p_sig);
			}
		}
	}
	if (diff < 0) {
		p_switchables_ptr_->erase(p_switchables_ptr_->begin() + *parity_level, p_switchables_ptr_->end());
	}
	
	ImGui::InputInt("Active Parity Index##SWITCH_parity_index", &parity_index_);
	clamp(&parity_index_, 0, *parity_level - 1);

	if (inspect_mode_) {
		switchables_ptr_ = &p_sig->switchables_[parity_index_];
	} else {
		switchables_ptr_ = &model_p_switchables_[parity_index_];
	}

	ImGui::Separator();

	for (int i = 0; i < *parity_level; ++i) {
		char buf[32];
		sprintf_s(buf, "Switchables %d", i);
		print_switchable_list(&(*p_switchables_ptr_)[i], buf);
		ImGui::Separator();
	}

	if (inspect_mode_) {
		if (ImGui::Button("Erase Selected Signaler##SWITCH")) {
			for (auto* s : p_sig->switches_) {
				s->remove_signaler(p_sig);
			}
			for (auto& group : p_sig->switchables_) {
				for (auto* s : group) {
					s->remove_signaler(p_sig);
				}
			}
			map->remove_signaler(p_sig);
			selected_sig = nullptr;
		}
	} else {
		if (ImGui::Button("Empty Queued Objects##SWITCH")) {
			model_switches_.clear();
			for (int i = 0; i < model_parity_level_; ++i) {
				model_p_switchables_[i].clear();
			}
		}
		if (ImGui::Button("Make Signaler##SWITCH")) {
			auto new_sig = std::make_unique<ParitySignaler>(model_label_, 0, model_parity_level_, false);
			for (auto* obj : model_switches_) {
				new_sig->push_switch(obj, true);
			}
			model_switches_.clear();
			for (int i = 0; i < model_parity_level_; ++i) {
				for (auto* obj : model_p_switchables_[i]) {
					new_sig->push_switchable(obj, true, i);
				}
				model_p_switchables_[i].clear();
			}
			map->push_signaler(std::move(new_sig));
		}
	}
}

void SwitchTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
	if (inspect_mode_ && !selected_sig) {
		return;
	}
	if (GameObject* obj = eroom->map()->view(pos)) {
		if (ObjectModifier* mod = obj->modifier()) {
			auto sw = dynamic_cast<Switch*>(mod);
			if (sw && std::find(switches_ptr_->begin(), switches_ptr_->end(), sw) == switches_ptr_->end()) {
				switches_ptr_->push_back(sw);
				if (inspect_mode_) {
					sw->push_signaler(selected_sig);
				}
				return;
			}
			auto swble = dynamic_cast<Switchable*>(mod);
			if (swble && std::find(switchables_ptr_->begin(), switchables_ptr_->end(), swble) == switchables_ptr_->end()) {
				switchables_ptr_->push_back(swble);
				if (inspect_mode_) {
					switch (sig_type_) {
					case SignalerType::Threshold:
						swble->push_signaler(selected_sig, 0);
						break;
					case SignalerType::Parity:
						swble->push_signaler(selected_sig, parity_index_);
						break;
					}
				}
				return;
			}
		}
	}
}

void SwitchTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
	if (inspect_mode_ && !selected_sig) {
		return;
	}
	if (GameObject* obj = eroom->map()->view(pos)) {
		ObjectModifier* mod = obj->modifier();
		if (auto sw = dynamic_cast<Switch*>(mod)) {
			erase_switch(sw, switches_ptr_);
		} else if (auto swble = dynamic_cast<Switchable*>(mod)) {
			erase_switchable(swble, switchables_ptr_);
		}
	}
}
