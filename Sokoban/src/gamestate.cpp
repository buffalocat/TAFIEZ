#include "stdafx.h"
#include "gamestate.h"

#include "graphicsmanager.h"

GameState::GameState() {}

GameState::GameState(GameState* parent): parent_{} {
	if (parent) {
		gfx_ = parent->gfx_;
		sound_ = parent->sound_;
		text_ = std::make_unique<TextRenderer>(gfx_->fonts_.get());
		window_ = parent->window_;
		current_state_ptr_ = parent->current_state_ptr_;
	}
}

GameState::~GameState() {}

void GameState::create_child(std::unique_ptr<GameState> child) {
	child->parent_ = std::move(*current_state_ptr_);
	*current_state_ptr_ = std::move(child);
}

void GameState::defer_to_parent() {
    *current_state_ptr_ = std::move(parent_);
}

void GameState::set_csp(std::unique_ptr<GameState>* csp) {
    current_state_ptr_ = csp;
}

void GameState::check_for_escape() {
    if (can_escape_quit_ && glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		handle_escape();
    } else if (!can_escape_quit_ && glfwGetKey(window_, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        can_escape_quit_ = true;
    }
}

void GameState::handle_escape() {
	if (parent_) {
		parent_->can_escape_quit_ = false;
	}
	defer_to_parent();
}

bool GameState::attempt_queued_quit() {
	if (queued_quit_ && can_quit(false)) {
		defer_to_parent();
		return true;
	}
	return false;
}

void GameState::queue_quit() {
	queued_quit_ = true;
}

bool GameState::can_quit(bool confirm) {
	return true;
}

void GameState::defer_to_sibling(std::unique_ptr<GameState> sibling) {
	sibling->parent_ = std::move(parent_);
	parent_ = std::move(sibling);
	queue_quit();
}
