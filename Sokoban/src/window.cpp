#include "stdafx.h"

#include "window.h"
#include "common_constants.h"
#include "graphicsmanager.h"


bool OpenGLWindow::init(int width, int height, const char* title) {
	LOG("Trying to init GLFW...");
	glfwInit();
	LOG("GLFW init'd");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, false);

	LOG("Trying to create GLFW window...");
	window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window_ == nullptr) {
		LOG("Failed to get a window");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window_);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		LOG("Failed to initialize GLAD");
		glfwTerminate();
		return false;
	}

	glfwSetWindowUserPointer(window_, this);
	glfwSetWindowSizeCallback(window_, OpenGLWindow::callback_resize);

	monitor_ = glfwGetPrimaryMonitor();
	glfwGetWindowSize(window_, &window_size_[0], &window_size_[1]);
	glfwGetWindowPos(window_, &window_pos_[0], &window_pos_[1]);
	update_viewport(nullptr);
	LOG("Setting some OpenGL things...");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	LOG("Made it to the end of window init!");
	return true;
}

void OpenGLWindow::callback_resize(GLFWwindow* window, int cx, int cy) {
	void* ptr = glfwGetWindowUserPointer(window);
	if (auto* window_ptr = static_cast<OpenGLWindow*>(ptr)) {
		window_ptr->resize(cx, cy);
	}
}

void OpenGLWindow::resize(int cx, int cy) {
	should_update_viewport_ = true;
}

bool OpenGLWindow::should_close() {
	return glfwWindowShouldClose(window_);
}

void OpenGLWindow::update_viewport(GraphicsManager* gfx) {
	int fb_w, fb_h;
	glfwGetFramebufferSize(window_, &fb_w, &fb_h);
	if (fb_w > fb_h * ASPECT_RATIO) {
		viewport_size_[0] = static_cast<int>(fb_h * ASPECT_RATIO);
		viewport_size_[1] = fb_h;
		viewport_pos_[0] = (fb_w - viewport_size_[0]) / 2;
		viewport_pos_[1] = 0;
	} else {
		viewport_size_[0] = fb_w;
		viewport_size_[1] = static_cast<int>(fb_w / ASPECT_RATIO);
		viewport_pos_[0] = 0;
		viewport_pos_[1] = (fb_h - viewport_size_[1]) / 2;
	}
	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window_);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);
	glViewport(viewport_pos_[0], viewport_pos_[1], viewport_size_[0], viewport_size_[1]);
	glScissor(viewport_pos_[0], viewport_pos_[1], viewport_size_[0], viewport_size_[1]);
	if (gfx) {
		gfx->generate_framebuffer();
	}
}

void OpenGLWindow::mainloop_before() {
	glfwPollEvents();
	if (should_update_viewport_) {
		update_viewport(nullptr);
		should_update_viewport_ = false;
	}
}

void OpenGLWindow::mainloop_after() {
	glfwSwapBuffers(window_);
}

void OpenGLWindow::set_fullscreen(bool fullscreen, GraphicsManager* gfx) {
	if (is_fullscreen() == fullscreen) {
		return;
	}
	if (fullscreen) {
		glfwGetWindowSize(window_, &window_size_[0], &window_size_[1]);
		glfwGetWindowPos(window_, &window_pos_[0], &window_pos_[1]);

		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwSetWindowMonitor(window_, monitor_, 0, 0, mode->width, mode->height, 0);
	} else {
		glfwSetWindowMonitor(window_, nullptr, window_pos_[0], window_pos_[1], window_size_[0], window_size_[1], 0);
	}
	update_viewport(gfx);
}

bool OpenGLWindow::is_fullscreen() {
	return glfwGetWindowMonitor(window_);
}

void OpenGLWindow::toggle_fullscreen(GraphicsManager* gfx) {
	set_fullscreen(!is_fullscreen(), gfx);
}