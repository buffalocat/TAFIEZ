#include "stdafx.h"
#include "gamestate.h"

#include "graphicsmanager.h"

GameState::GameState() {}

GameState::~GameState() {}

void GameState::create_child(std::unique_ptr<GameState> child) {
    child->parent_ = std::move(*current_state_ptr_);
    child->gfx_ = gfx_;
    child->window_ = window_;
    child->current_state_ptr_ = current_state_ptr_;
    *current_state_ptr_ = std::move(child);
}

void GameState::defer_to_parent() {
    *current_state_ptr_ = std::move(parent_);
}

void GameState::set_graphics(GraphicsManager* gfx) {
    gfx_ = gfx;
    window_ = gfx->window();
}

void GameState::set_csp(std::unique_ptr<GameState>* csp) {
    current_state_ptr_ = csp;
}

void GameState::check_for_escape_quit() {
    if (can_escape_quit_ && glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (parent_) {
            parent_->can_escape_quit_ = false;
        }
        defer_to_parent();
    } else if (!can_escape_quit_ && glfwGetKey(window_, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        can_escape_quit_ = true;
    }
}

bool GameState::check_for_queued_quit() {
	if (queued_quit_) {
		defer_to_parent();
		return true;
	}
	return false;
}

void GameState::queue_quit() {
	queued_quit_ = true;
}