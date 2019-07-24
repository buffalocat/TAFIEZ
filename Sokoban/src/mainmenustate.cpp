#include "stdafx.h"
#include "mainmenustate.h"

#include "editorstate.h"
#include "realplayingstate.h"

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
			//create_child(std::make_unique<RealPlayingState>("Default", {0,0,0}));
		}
		if (ImGui::Button("Delete File##MAINMENU")) {
			menu_type_ = Menu::Delete;
		}
		break;
	case Menu::New:
		if (ImGui::Button("Start New Game on File 1##MAINMENU")) {
			// Create new file here
			create_child(std::make_unique<RealPlayingState>("1", "story1"));
		}
		if (ImGui::Button("Back##MAINMENU")) {
			menu_type_ = Menu::Top;
		}
		break;
	case Menu::Load:
		if (ImGui::Button("Load Game from File 1##MAINMENU")) {
			// load file here
			create_child(std::make_unique<RealPlayingState>("1", "story1"));
		}
		if (ImGui::Button("Back##MAINMENU")) {
			menu_type_ = Menu::Top;
		}
		break;
	case Menu::Delete:
		if (ImGui::Button("Delete Save File 1?##MAINMENU")) {
			// Delete the file (ARE YOU SURE??)
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
