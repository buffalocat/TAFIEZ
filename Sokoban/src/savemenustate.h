#pragma once

#include "gamestate.h"

class RealPlayingState;
class SaveProfile;
class StringDrawer;
class Font;
class GameState;

using MenuCallback = std::function<void(void)>;

class ImageDrawer {};

struct SaveSlot {
	std::unique_ptr<StringDrawer> text{};
	MenuCallback callback{};
	std::unique_ptr<ImageDrawer> image{};
};


class LoadMenuState : public GameState {
public:
	LoadMenuState(GameState* parent);
	~LoadMenuState();

	void handle_entry_select_input();
	void handle_input(GameState* game_state);
	void update();
	void draw();
	void main_loop();

private:
	RealPlayingState* rps_;
	SaveProfile* savefile_;

	Font* font_;
	Font* big_font_;

	SaveSlot backup_save_;
	std::vector<SaveSlot> auto_saves_;
	std::vector<SaveSlot> manual_saves_;

	void generate_callbacks();
	void generate_text();

	void move_x(int dir);
	void move_y(int dir);

	void set_cur_save_slot();

	int menu_x_ = 0;
	int menu_y_ = 0;

	std::unique_ptr<StringDrawer> return_option_;

	StringDrawer* active_string_drawer_{};
	StringDrawer* prev_active_sd_{};
	std::vector<StringDrawer*> string_drawers_{};
	SaveSlot* cur_slot_{};
	float scale_ = 1.0f;
	float top_ = 0.4f;
	float v_space_ = 0.2f;
	glm::vec4 inactive_color_, active_color_;
	int time_;
	int input_cooldown_ = 0;
	bool can_select_ = false;
};