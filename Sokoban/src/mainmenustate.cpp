#include "stdafx.h"
#include "mainmenustate.h"

#include "common_constants.h"
#include "editorstate.h"
#include "realplayingstate.h"
#include "savefile.h"

MainMenuState::MainMenuState() : GameState(), menu_type_{ Menu::Top } {}

MainMenuState::~MainMenuState() {}

void MainMenuState::main_loop() {
	bool p_open = true;
	if (!ImGui::Begin("Main Menu Window##MAINMENU", &p_open)) {
		ImGui::End();
		return;
	}

	switch (menu_type_) {
	case Menu::Top:
		if (ImGui::Button("Open Editor##MAINMENU")) {
			create_child(std::make_unique<EditorState>(gfx_));
		}
		if (ImGui::Button("Start New File##MAINMENU")) {
			menu_type_ = Menu::New;
		}
		if (ImGui::Button("Load File##MAINMENU")) {
			menu_type_ = Menu::Load;
		}
		if (ImGui::Button("Delete File##MAINMENU")) {
			menu_type_ = Menu::Delete;
		}
		break;
	case Menu::New:
		if (ImGui::Button("Start New Game on File 1##MAINMENU")) {
			auto savefile_dir = "1";
			auto savefile = std::make_unique<SaveFile>(savefile_dir);
			if (savefile->create()) {
				auto playing_state_unique = std::make_unique<RealPlayingState>(std::move(savefile));
				auto playing_state = playing_state_unique.get();
				create_child(std::move(playing_state_unique));
				playing_state->start_from_map(NEW_FILE_START_MAP);
			} else {
				std::cout << "That file already exists! Load it or delete it!" << std::endl;
			}
		}
		if (ImGui::Button("Back##MAINMENU")) {
			menu_type_ = Menu::Top;
		}
		break;
	case Menu::Load:
		if (ImGui::Button("Load Game from File 1##MAINMENU")) {
			auto savefile_dir = "1";
			auto savefile = std::make_unique<SaveFile>(savefile_dir);
			if (savefile->load_meta()) {
				auto playing_state_unique = std::make_unique<RealPlayingState>(std::move(savefile));
				auto playing_state = playing_state_unique.get();
				create_child(std::move(playing_state_unique));
				playing_state->load_most_recent_subsave();
			} else {
				std::cout << "That doesn't exist!!" << std::endl;
			}
		}
		if (ImGui::Button("Back##MAINMENU")) {
			menu_type_ = Menu::Top;
		}
		break;
	case Menu::Delete:
		if (ImGui::Button("Delete Save File 1?##MAINMENU")) {
			// Delete the file (ARE YOU SURE??)
			std::filesystem::remove_all(std::filesystem::path("saves") / "1");
		}
		if (ImGui::Button("Back##MAINMENU")) {
			menu_type_ = Menu::Top;
		}
		break;
	default:
		break;
	}    
    ImGui::End();
}
