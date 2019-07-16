#include "stdafx.h"
#include "mainmenustate.h"


#include "editorstate.h"

MainMenuState::MainMenuState(): GameState() {}

MainMenuState::~MainMenuState() {}

void MainMenuState::main_loop() {
    bool p_open = true;
    if (!ImGui::Begin("Main Menu Window##MAINMENU", &p_open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Open Editor##MAINMENU")) {
        create_child(std::make_unique<EditorState>(gfx_));
    }
    if (ImGui::Button("Start New File##MAINMENU")) {
        //create_child(std::make_unique<PlayingState>("Default", {0,0,0}));
    }
    if (ImGui::Button("Load File##MAINMENU")) {
        //create_child(std::make_unique<RealPlayingState>("Default", {0,0,0}));
    }
    ImGui::End();
}
