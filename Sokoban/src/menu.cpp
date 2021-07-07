#include "stdafx.h"
#include "menu.h"

#include "gamestate.h"
#include "stringdrawer.h"
#include "fontmanager.h"

#include "color_constants.h"
#include "common_constants.h"


Menu::Menu(GameState* state, Font* font) : state_{ state }, font_ { font } {
	active_color_ = COLOR_VECTORS[GOLD];
	inactive_color_ = COLOR_VECTORS[BLUE];
}

Menu::~Menu() {}

void Menu::push_entry(std::string label, MenuCallback callback) {
	glm::vec4 color = inactive_color_;
	if (num_entries_ == 0) {
		color = active_color_;
	}
	entries_.push_back(MenuEntry{
		std::make_unique<StringDrawer>(font_, color,
			label, 0.0f, top_ - v_space_ * num_entries_, scale_, scale_, 0.0f),
		callback });
	++num_entries_;
}

void Menu::set_current_entry() {

}

const int MAX_MENU_COOLDOWN = 8;
const int MENU_CYCLE_TIME = 120;

int Menu::handle_entry_select_input() {
	can_select_ = true;
	int new_current = current_;
	if (state_->key_pressed(GLFW_KEY_UP)) {
		if (input_cooldown_ == 0) {
			--new_current;
			if (new_current < 0) {
				new_current += num_entries_;
			}
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else if (state_->key_pressed(GLFW_KEY_DOWN)) {
		if (input_cooldown_ == 0) {
			++new_current;
			if (new_current >= num_entries_) {
				new_current -= num_entries_;
			}
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else {
		input_cooldown_ = 0;
	}
	return new_current;
}

void Menu::handle_input() {
	if (num_entries_ == 0) {
		return;
	}
	int new_current = current_;
	if (state_->key_pressed(GLFW_KEY_ENTER)) {
		if (can_select_) {
			can_select_ = false;
			(entries_[current_].callback)();
		}
	} else {
		new_current = handle_entry_select_input();
	}
	set_current_entry(new_current);
}

void Menu::set_current_entry(int new_current) {
	if (current_ == new_current) {
		return;
	}
	entries_[current_].text->set_color(inactive_color_);
	current_ = new_current;
}



void Menu::update() {
	++time_;
	glm::vec4 cur_color = active_color_ * glm::vec4((float)(0.9f + 0.15f * sin(time_ * TWO_PI / (float)MENU_CYCLE_TIME)));
	cur_color.w = 1.0f;
	entries_[current_].text->set_color(cur_color);
	if (time_ == MENU_CYCLE_TIME) {
		time_ = 0;
	}
}

void Menu::draw() {
	font_->shader_->use();
	glDisable(GL_DEPTH_TEST);
	for (auto& entry : entries_) {
		entry.text->render();
	}
}
