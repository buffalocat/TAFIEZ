#pragma once

struct GLFWmonitor;
struct GLFWwindow;
class GraphicsManager;

class OpenGLWindow {
public:
	bool init(int width, int height, const char* title);
	static void callback_resize(GLFWwindow* window, int cx, int cy);
	bool is_fullscreen(void);
	void set_fullscreen(bool fullscreen, GraphicsManager* gfx);
	void toggle_fullscreen(GraphicsManager* gfx);
	bool should_close();
	void update_viewport(GraphicsManager* gfx);
	void mainloop_before();
	void mainloop_after();

	GLFWwindow* window_{};
	std::array<int, 2> window_pos_{ 0, 0 };
	std::array<int, 2> window_size_{ 0, 0 };
	std::array<int, 2> viewport_pos_{ 0, 0 };
	std::array<int, 2> viewport_size_{ 0, 0 };

private:
	bool should_update_viewport_ = true;
	GLFWmonitor* monitor_{};

	void resize(int cx, int cy);
};