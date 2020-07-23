#ifndef SOKOBAN_MENU_H
#define SOKOBAN_MENU_H

class StringDrawer;
class Font;
class GameState;

using MenuCallback = std::function<void(void)>;

struct MenuEntry {
	std::unique_ptr<StringDrawer> text;
	MenuCallback callback;
};


class Menu {
public:
	Menu(GLFWwindow* window, Font* font);
	~Menu();

	void push_entry(std::string label, MenuCallback callback);
	void set_current_entry();

	int handle_entry_select_input();
	void handle_input(GameState* game_state);
	void set_current_entry(int new_current);
	void update();
	void draw();

	int num_entries_ = 0;

private:
	GLFWwindow* window_;
	Font* font_;
	std::vector<MenuEntry> entries_{};
	int current_ = 0;

	float scale_ = 1.0f;
	float top_ = 0.4f;
	float v_space_ = 0.2f;
	glm::vec4 inactive_color_, active_color_;
	int time_;
	int input_cooldown_ = 0;
	bool can_select_ = false;
};

#endif //SOKOBAN_MENU_H