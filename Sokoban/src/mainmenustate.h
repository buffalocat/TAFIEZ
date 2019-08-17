#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "gamestate.h"

enum class Menu {
	Top,
	New,
	Load,
	Delete,
};

class MainMenuState: public GameState {
public:
	MainMenuState();
    MainMenuState(GameState* parent);
    ~MainMenuState();
    void main_loop();

private:
	Menu menu_type_;
};

#endif // MAINMENUSTATE_H
