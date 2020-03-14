#include "stdafx.h"
#include "mainmenustate.h"

#include "common_constants.h"
#include "editorstate.h"
#include "realplayingstate.h"
#include "savefile.h"
#include "menu.h"
#include "graphicsmanager.h"
#include "fontmanager.h"

constexpr bool EDITOR_ENABLED = true;

MainMenuState::MainMenuState(GameState* parent) : GameState(parent) {}

MainMenuState::~MainMenuState() {}

void MainMenuState::init_menu() {
	menu_ = std::make_unique<Menu>(window_, gfx_->fonts_->get_font(Fonts::ABEEZEE, 72));
	if (EDITOR_ENABLED) {
		menu_->push_entry("Open Editor", [this]() { create_child(std::make_unique<EditorState>(this)); });
	}
	menu_->push_entry("Select File", [this]() { open_file_select(); });
	menu_->push_entry("Quit", [this]() { queue_quit(); });
}

void MainMenuState::main_loop() {
	menu_->update();
	menu_->handle_input(this);
	draw();
}

void MainMenuState::open_file_select() {
	create_child(std::make_unique<FileSelectState>(this));
}

void MainMenuState::draw() {
	menu_->draw();
}


const int MAX_SAVE_FILES = 7;

FileSelectState::FileSelectState(GameState* parent) : GameState(parent) {
	load_save_info();
	create_menu();
}

FileSelectState::~FileSelectState() {}

void FileSelectState::main_loop() {
	menu_->update();
	menu_->handle_input(this);
	draw();
}

void FileSelectState::load_save_info() {
	for (int i = 0; i < MAX_SAVE_FILES; ++i) {
		std::string cur_name = std::to_string(i + 1);
		auto cur_file = std::make_unique<SaveFile>(cur_name);
		cur_file->create_save_dir();
		cur_file->load_meta();
		if (cur_file->exists_) {
			cur_file->load_most_recent_subsave();
		}
		save_files_.push_back(std::move(cur_file));
	}
}

void FileSelectState::create_menu() {
	menu_ = std::make_unique<Menu>(window_, gfx_->fonts_->get_font(Fonts::ABEEZEE, 72));
	if (save_files_[save_index_]->exists_) {
		menu_->push_entry("Continue", [this]() { continue_file(); });
		menu_->push_entry("Load Save", [this]() {});
		menu_->push_entry("Delete File", [this]() {});
	} else {
		menu_->push_entry("New File", [this]() { new_file(); });
	}
	menu_->push_entry("Return to Main Menu", [this]() { queue_quit(); });
}

void FileSelectState::draw() {
	menu_->draw();
}

void FileSelectState::new_file() {
	auto playing_state_unique = std::make_unique<RealPlayingState>(save_files_[save_index_].get(), this);
	auto playing_state = playing_state_unique.get();
	create_child(std::move(playing_state_unique));
	playing_state->play_from_map(NEW_FILE_START_MAP);
}

void FileSelectState::continue_file() {
	auto playing_state_unique = std::make_unique<RealPlayingState>(save_files_[save_index_].get(), this);
	auto playing_state = playing_state_unique.get();
	create_child(std::move(playing_state_unique));
	playing_state->play_from_loaded_subsave();
}