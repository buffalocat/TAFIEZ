#include "stdafx.h"
#include "menu.h"

#include "gamestate.h"
#include "stringdrawer.h"
#include "fontmanager.h"

#include "color_constants.h"
#include "common_constants.h"


Menu::Menu(GLFWwindow* window, Font* font) : window_{ window }, font_{ font } {
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

const int MAX_MENU_COOLDOWN = 8;
const int MENU_CYCLE_TIME = 120;

void Menu::handle_input(GameState* game_state) {
	if (num_entries_ == 0) {
		return;
	}
	entries_[current_].text->set_color(inactive_color_);
	if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS) {
		if (can_select_) {
			can_select_ = false;
			(entries_[current_].callback)();
		}
	} else {
		can_select_ = true;
		if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS) {
			if (input_cooldown_ == 0) {
				--current_;
				if (current_ < 0) {
					current_ += num_entries_;
				}
				time_ = 0;
				input_cooldown_ = MAX_MENU_COOLDOWN;
			} else {
				--input_cooldown_;
			}
		} else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS) {
			if (input_cooldown_ == 0) {
				++current_;
				if (current_ >= num_entries_) {
					current_ -= num_entries_;
				}
				time_ = 0;
				input_cooldown_ = MAX_MENU_COOLDOWN;
			} else {
				--input_cooldown_;
			}
		} else {
			input_cooldown_ = 0;
		}
	}
	glm::vec4 cur_color = active_color_ * glm::vec4((float)(0.85f + 0.2f * sin(time_ * TWO_PI  / (float)MENU_CYCLE_TIME)));
	cur_color.w = 1.0f;
	entries_[current_].text->set_color(cur_color);
}

void Menu::update() {
	++time_;
	if (time_ == MENU_CYCLE_TIME) {
		time_ = 0;
	}
}

void Menu::draw() {
	font_->shader_->use();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	for (auto& entry : entries_) {
		entry.text->render();
	}
}