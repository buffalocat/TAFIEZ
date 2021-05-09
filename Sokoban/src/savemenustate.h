#pragma once

#include "gamestate.h"

class RealPlayingState;
class SaveProfile;
class StringDrawer;
class Font;
class GameState;

using LoadCallback = std::function<bool(void)>;

class ImageDrawer {};

struct SaveSlotLoader {
	std::unique_ptr<StringDrawer> text{};
	LoadCallback callback{};
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

	SaveSlotLoader backup_save_;
	std::vector<SaveSlotLoader> auto_saves_;
	std::vector<SaveSlotLoader> manual_saves_;

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
	SaveSlotLoader* cur_slot_{};
	float scale_ = 1.0f;
	float top_ = 0.4f;
	float v_space_ = 0.2f;
	glm::vec4 inactive_color_, active_color_;
	int time_;
	int input_cooldown_ = 0;
	bool can_select_ = false;
};



using SaveCallback = std::function<void(void)>;

struct SaveSlotSaver {
	std::unique_ptr<StringDrawer> text{};
	SaveCallback callback{};
	std::unique_ptr<ImageDrawer> image{};
};


class SaveMenuState : public GameState {
public:
	SaveMenuState(GameState* parent);
	~SaveMenuState();

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

	std::vector<SaveSlotSaver> manual_saves_;

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
	SaveSlotSaver* cur_slot_{};
	float scale_ = 1.0f;
	float top_ = 0.4f;
	float v_space_ = 0.2f;
	glm::vec4 inactive_color_, active_color_;
	int time_;
	int input_cooldown_ = 0;
	bool can_select_ = false;
};

