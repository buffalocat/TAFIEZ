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

	void handle_input(GameState* game_state);
	void update();
	void draw();

private:
	GLFWwindow* window_;
	Font* font_;
	std::vector<MenuEntry> entries_{};
	int num_entries_ = 0;
	int current_ = 0;

	float scale_ = 1.0f;
	float top_ = 0.4f;
	float v_space_ = 0.2f;
	glm::vec4 inactive_color_, active_color_;
	int time_;
};

#endif //SOKOBAN_MENU_H