// Sokoban.cpp : Defines the entry point for the application.

#include "stdafx.h"

#include <stdexcept>

#ifdef SOKOBAN_EDITOR
#include <dear/imgui_impl_glfw.h>
#include <dear/imgui_impl_opengl3.h>
#include "editorstate.h"
#endif

#include "common_constants.h"
#include "graphicsmanager.h"
#include "soundmanager.h"
#include "gamestate.h"
#include "fontmanager.h"
#include "mainmenustate.h"
#include "fpstimer.h"
#include "savefile.h"
#include "window.h"

#include <cstdio>

void init_log_file(const char* file_name) {
	freopen(file_name, "w", stdout);
}

int main() {
#ifdef SOKOBAN_DEBUG
	init_log_file("output.txt");
#endif

	LOG("First line of main!");

	OpenGLWindow window;
	if (!window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "There's a Flag in Each Zone")) {
		LOG("Window init failed?!");
		return -1;
	}

	LOG("Looks like window init worked!");

#ifdef SOKOBAN_EDITOR
	// ImGui init
	const char* glsl_version = "#version 330";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(window.window_, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGui::StyleColorsDark();

	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
#endif

	GraphicsManager gfx(&window);
	SoundManager sound{};

	for (int i : {48, 72, 108}) {
		gfx.fonts_->get_font(Fonts::ABEEZEE, i);
	}

	LOG("Loaded some fonts and whatever!");

	std::unique_ptr<GameState> current_state = std::make_unique<MainMenuState>(&gfx, &sound, &window);
	current_state->set_csp(&current_state);
	

	auto main_menu = static_cast<MainMenuState*>(current_state.get());

	LOG("Created the main menu!");

	FPSTimer timer{60};
	gfx.generate_framebuffer();

	LOG("About to enter the main loop...");

	unsigned int frame_count = 0;

	while (!window.should_close()) {
		if (auto error = glGetError()) {
			LOG("OpenGL Error detected at start of main loop! " << error);
		}
		auto time_before_frame = std::chrono::high_resolution_clock::now();

		window.mainloop_before();

#ifdef SOKOBAN_EDITOR
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#endif

		if (!current_state) {
			break;
		}

		// Check for manual quit
		current_state->check_for_escape();
		// Check for queued quits until we're done or reach the top
		while (current_state) {
			if (!current_state->attempt_queued_quit()) {
				break;
			}
		}
		if (!current_state) {
			break;
		}

		current_state->main_loop();

#ifdef SOKOBAN_EDITOR
		static bool show_demo_window = false;
		if (!ImGui::GetIO().WantCaptureKeyboard) {
			if (glfwGetKey(window.window_, GLFW_KEY_F1)) {
				show_demo_window = true;
			}
		}

		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
		window.mainloop_after();
		timer.wait_for_frame_end();
		frame_count++;
	}
	LOG("Exited the main loop. It ran for " << frame_count << " frames.");
	glfwTerminate();
	LOG("GLFW terminated.");
	return 0;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	main();
	return 0;
}