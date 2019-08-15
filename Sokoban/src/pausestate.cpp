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

template <class State>
Menu<State>::Menu(GLFWwindow* window, Font* font) : window_{ window }, font_{ font } {}

template <class State>
Menu<State>::~Menu() {}

template <class State>
void Menu<State>::push_entry(std::string label, MenuCallback<State> callback) {
	int color = inactive_color_;
	if (num_entries_ == 0) {
		color = active_color_;
	}
	entries_.push_back(MenuEntry<State>{
		std::make_unique<StringDrawer>(font_, COLOR_VECTORS[color],
			label, 0.0f, top_ - v_space_ * num_entries_, scale_, scale_),
		callback });
	++num_entries_;
}

const int MAX_MENU_COOLDOWN = 8;

template <class State>
void Menu<State>::handle_input(State* game_state) {
	if (num_entries_ == 0) {
		return;
	}
	static int input_cooldown = 0;
	if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS) {
		if (input_cooldown == 0) {
			entries_[current_].text->set_color(inactive_color_);
			--current_;
			if (current_ < 0) {
				current_ += num_entries_;
			}
			entries_[current_].text->set_color(active_color_);
			input_cooldown = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown;
		}
	} else if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (input_cooldown == 0) {
			entries_[current_].text->set_color(inactive_color_);
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
		(game_state->*entries_[current_].callback)();
	} else {
		input_cooldown = 0;
	}
}

template <class State>
void Menu<State>::draw() {
	font_->shader_.use();
	for (auto& entry : entries_) {
		entry.text->render();
	}
}


PauseState::PauseState(GraphicsManager* gfx, PlayingState* parent, PlayingGlobalData* global) : GameState(),
playing_state_{ parent },
menu_{ gfx->window(), gfx->fonts_->get_font(Fonts::KALAM_BOLD, 72) } {
	menu_.push_entry("Unpause", &PauseState::unpause);
	if (dynamic_cast<RealPlayingState*>(parent)) {
		menu_.push_entry("Save Game", &PauseState::save);
		if (global->has_flag(WORLD_RESET_GLOBAL_ID)) {
			menu_.push_entry("World Reset", &PauseState::world_reset);
		}
		menu_.push_entry("Quit Game", &PauseState::quit);
	} else if (dynamic_cast<TestPlayingState*>(parent)) {
		menu_.push_entry("Quit Test Session", &PauseState::quit);
	}
}

PauseState::~PauseState() {}


void PauseState::main_loop() {
	menu_.handle_input(this);
	draw();
}

void PauseState::draw() {
	//draw_paused_game();
	menu_.draw();
}

void PauseState::unpause() {
	queue_quit();
}

void PauseState::save() {
	playing_state_->make_subsave();
}

void PauseState::quit() {
	//TODO: "Are you sure?"
	queue_quit();
	playing_state_->queue_quit();
}

void PauseState::world_reset() {
	playing_state_->world_reset();
	unpause();
}

// Won't do quite the right thing, because some objects aren't in draw_world()
void PauseState::draw_paused_game() {
	gfx_->prepare_object_rendering();
	gfx_->draw_world();
}