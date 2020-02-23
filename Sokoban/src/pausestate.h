#pragma once
#include "gamestate.h"

class StringDrawer;
class Font;
class PlayingState;

template <class State>
using MenuCallback = void(State::*)();

template <class State>
struct MenuEntry {

	std::unique_ptr<StringDrawer> text;
	MenuCallback<State> callback;
};

template <class State>
class Menu {
public:
	Menu(GLFWwindow* window, Font* font);
	~Menu();

	void push_entry(std::string label, MenuCallback<State> callback);

	void handle_input(State* game_state);
	void draw();

private:
	GLFWwindow* window_;
	Font* font_;
	std::vector<MenuEntry<State>> entries_{};
	int num_entries_ = 0;
	int current_ = 0;

	float scale_ = 1.0f;
	float top_ = 0.4f;
	float v_space_ = 0.2f;
	int inactive_color_ = BLUE;
	int active_color_ = GOLD;
};

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
	Menu<PauseState> menu_;
};

