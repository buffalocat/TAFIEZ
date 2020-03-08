#include "stdafx.h"
#include "pausestate.h"

#include "graphicsmanager.h"
#include "fontmanager.h"
#include "stringdrawer.h"
#include "playingstate.h"
#include "savefile.h"
#include "common_constants.h"
#include "realplayingstate.h"
#include "testplayingstate.h"

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
	static int input_cooldown = 0;
	if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS) {
		if (input_cooldown == 0) {
			--current_;
			if (current_ < 0) {
				current_ += num_entries_;
			}
			time_ = 0;
			input_cooldown = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown;
		}
	} else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (input_cooldown == 0) {
			++current_;
			if (current_ >= num_entries_) {
				current_ -= num_entries_;
			}
			entries_[current_].text->set_color(active_color_);
			input_cooldown = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown;
		}
	} else if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS) {
		input_cooldown = MAX_MENU_COOLDOWN;
		(entries_[current_].callback)();
	} else {
		input_cooldown = 0;
	}
	glm::vec4 cur_color = active_color_ * glm::vec4((float)(0.85f + 0.3f * cos(time_ * TWO_PI  / (float)MENU_CYCLE_TIME)));
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


PauseState::PauseState(GameState* parent) : GameState(parent),
playing_state_{ static_cast<PlayingState*>(parent) },
menu_{ std::make_unique<Menu>(window_, gfx_->fonts_->get_font(Fonts::ABEEZEE, 72)) } {
	menu_->push_entry("Unpause", [this]() { unpause(); });
	if (dynamic_cast<RealPlayingState*>(parent)) {
		menu_->push_entry("Save Game", [this]() { playing_state_->make_subsave(); });
		if (playing_state_->global_->has_flag(WORLD_RESET_GLOBAL_ID)) {
			menu_->push_entry("World Reset", [this]() { world_reset(); });
		}
		menu_->push_entry("Quit Game", [this]() { quit_playing(); });
	} else if (dynamic_cast<TestPlayingState*>(parent)) {
		menu_->push_entry("Quit Test Session", [this]() { quit_playing(); });
	}
}

PauseState::~PauseState() {}


void PauseState::main_loop() {
	menu_->update();
	menu_->handle_input(this);
	draw();
}

void PauseState::draw() {
	double prev_shadow = gfx_->shadow_;
	gfx_->shadow_ *= 0.3;
	gfx_->post_rendering();
	gfx_->shadow_ = prev_shadow;
	menu_->draw();
}

void PauseState::unpause() {
	queue_quit();
}

void PauseState::quit_playing() {
	queue_quit();
	playing_state_->queue_quit();
}

void PauseState::world_reset() {
	playing_state_->world_reset();
	unpause();
}
