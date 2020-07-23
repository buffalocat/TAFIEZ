#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "gamestate.h"

class Menu;
class SaveFile;

class MainMenuState: public GameState {
public:
    MainMenuState(GameState* parent);
    ~MainMenuState();
    void main_loop();
	void init_menu();
	void handle_escape();

	void open_file_select();
	void draw();

private:
	std::unique_ptr<Menu> menu_;
	int quit_entry_index_;
};


class FileSelectState : public GameState {
public:
	FileSelectState(GameState* parent);
	~FileSelectState();
	void main_loop();
	void load_save_info();
	void create_menu();
	void draw();

	void new_file();
	void continue_file();

private:
	int save_index_;
	std::unique_ptr<Menu> menu_{};
	std::vector<std::unique_ptr<SaveFile>> save_files_{};
};

#endif // MAINMENUSTATE_H
