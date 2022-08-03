#pragma once

#include "gamestate.h"

class Menu;

class OptionMenuState: public GameState {
public:
	OptionMenuState(GameState* parent);
	~OptionMenuState();

private:
	std::unique_ptr<Menu> menu_;
};
