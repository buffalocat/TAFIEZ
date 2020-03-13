#pragma once
#include "gamestate.h"

class Menu;
class PlayingState;

class PauseState : public GameState {
public:
	PauseState(GameState* parent);
	~PauseState();

	void main_loop();

private:
	void draw();

	//Menu Callbacks
	void unpause();
	void save();
	void world_reset();
	void quit_playing();

	PlayingState* playing_state_;
	std::unique_ptr<Menu> menu_;
};

