#include "stdafx.h"
#include "mainmenustate.h"

#include "common_constants.h"
#include "editorstate.h"
#include "realplayingstate.h"
#include "savefile.h"
#include "menu.h"
#include "graphicsmanager.h"
#include "fontmanager.h"
#include "window.h"
#include "stringdrawer.h"


MainMenuState::MainMenuState(GameState* parent) : GameState(parent) {
	init_menu();
}

MainMenuState::MainMenuState(GraphicsManager* gfx, SoundManager* sound, OpenGLWindow* window) :
	GameState() {
	gfx_ = gfx;
	sound_ = sound;
	window_ = window;
	text_ = std::make_unique<TextRenderer>(gfx->fonts_.get());
	init_menu();
}

MainMenuState::~MainMenuState() {}

void MainMenuState::init_menu() {
	menu_ = std::make_unique<Menu>(this, gfx_->fonts_->get_font(Fonts::ABEEZEE, 72));
#ifdef SOKOBAN_EDITOR
	menu_->push_entry("Open Editor", [this]() { create_child(std::make_unique<EditorState>(this)); });
#endif
	menu_->push_entry("Select Profile", [this]() { open_file_select(); });
	menu_->push_entry("Toggle Fullscreen", [this]() { toggle_fullscreen(); });
	quit_entry_index_ = menu_->num_entries_;
	menu_->push_entry("Quit", [this]() { queue_quit(); });
}

void MainMenuState::main_loop() {
	menu_->update();
	menu_->handle_input();
	draw();
}

void MainMenuState::toggle_fullscreen() {
	window_->toggle_fullscreen(gfx_);
	defer_to_sibling(std::make_unique<MainMenuState>(this));
}

void MainMenuState::open_file_select() {
	create_child(std::make_unique<FileSelectState>(this));
}

void MainMenuState::draw() {
	gfx_->clear_screen(CLEAR_COLOR);
	menu_->draw();
}


void MainMenuState::handle_escape() {
	menu_->set_current_entry(quit_entry_index_);
}

const int MAX_SAVE_FILES = 1;

FileSelectState::FileSelectState(GameState* parent) : GameState(parent) {
	load_save_info();
	create_menu();
}

FileSelectState::~FileSelectState() {}

void FileSelectState::main_loop() {
	menu_->update();
	menu_->handle_input();
	draw();
}

void FileSelectState::load_save_info() {
	for (int i = 0; i < MAX_SAVE_FILES; ++i) {
		std::string cur_name = std::to_string(i + 1);
		auto cur_file = std::make_unique<SaveProfile>(cur_name);
		cur_file->create_save_dir();
		cur_file->load_meta();
		cur_file->load_global();
		save_files_.push_back(std::move(cur_file));
	}
}

void FileSelectState::create_menu() {
	menu_ = std::make_unique<Menu>(this, gfx_->fonts_->get_font(Fonts::ABEEZEE, 72));
	if (save_files_[save_index_]->exists_) {
		menu_->push_entry("Continue", [this]() { continue_file(); });
//		menu_->push_entry("Load Save", [this]() {});
//		menu_->push_entry("Delete File", [this]() {});
	} else {
		menu_->push_entry("New File", [this]() { new_file(); });
	}
	menu_->push_entry("Return to Main Menu", [this]() { queue_quit(); });
}

void FileSelectState::draw() {
	gfx_->clear_screen(CLEAR_COLOR);
	menu_->draw();
}

void FileSelectState::new_file() {
	auto* cur_file = save_files_[save_index_].get();
		auto playing_state_unique = std::make_unique<RealPlayingState>(cur_file, this);
	auto playing_state = playing_state_unique.get();
	create_child(std::move(playing_state_unique));
	playing_state->play_from_map("T");
	queue_quit();
}

void FileSelectState::continue_file() {
	auto* cur_file = save_files_[save_index_].get();
	auto playing_state_unique = std::make_unique<RealPlayingState>(cur_file, this);
	auto playing_state = playing_state_unique.get();
	create_child(std::move(playing_state_unique));
	cur_file->load_subsave_dispatch(SaveType::Emergency, 0, playing_state);
	playing_state->play_from_loaded_subsave();
	queue_quit();
}