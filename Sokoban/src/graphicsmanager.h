#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "color_constants.h"
#include "modelinstancer.h"

struct GLFWwindow;
class FontManager;
class AnimationManager;
class Animation;
class StringDrawer;
class PlayingState;

class ProtectedStringDrawer {
public:
	ProtectedStringDrawer(StringDrawer* drawer);
	~ProtectedStringDrawer();
	ProtectedStringDrawer(ProtectedStringDrawer&) = delete;
	ProtectedStringDrawer& operator=(ProtectedStringDrawer&) = delete;
	ProtectedStringDrawer(ProtectedStringDrawer&&);
	ProtectedStringDrawer& operator=(ProtectedStringDrawer&&) = delete;

	StringDrawer* drawer_;
	bool alive_;
};


enum class GraphicsState {
	None,
	FadeOut,
	Black,
	FadeIn,
};


class GraphicsManager {
public:
	GraphicsManager(GLFWwindow*);
	~GraphicsManager();

	GLFWwindow* window();

	void update();
	void set_state(GraphicsState state);
	bool in_animation();

	void set_PV(glm::mat4, glm::mat4);
	void set_light_source(glm::vec3);

	void pre_object_rendering();
	void prepare_draw_objects();
	void prepare_draw_objects_particle_atlas(GLuint atlas);
	void draw_objects();
	void pre_particle_rendering();
	void post_rendering();
	glm::vec3 view_dir();

	ModelInstancer cube{ "resources/uniform_cube.obj" };
	ModelInstancer top_cube{ "resources/top_cube.obj" };
	ModelInstancer six_squares{ "resources/six_squares.obj" };
	ModelInstancer windshield{ "resources/windshield.obj" };

	ModelInstancer diamond{ "resources/diamond.obj" };
	ModelInstancer top_diamond{ "resources/top_diamond.obj" };
	ModelInstancer six_squares_diamond{ "resources/six_squares_diamond.obj" };
	ModelInstancer windshield_diamond{ "resources/windshield_diamond.obj" };

	ModelInstancer cube_edges{ "resources/cube_edges.obj" };
	ModelInstancer flag{ "resources/flag.obj" };
	
	// Horizontal square, facing up
	ModelInstancer square_flat{ "resources/square_flat.obj" };
	// Vertical squares, facing each of the cardinal directions
	ModelInstancer square_0{ "resources/square_0.obj" };
	ModelInstancer square_1{ "resources/square_1.obj" };
	ModelInstancer square_2{ "resources/square_2.obj" };
	ModelInstancer square_3{ "resources/square_3.obj" };

	std::unique_ptr<FontManager> fonts_{};

	glm::mat4 proj_, view_;
	glm::vec3 light_source_;
	double shadow_;

	Shader instanced_shader_{ Shader("shaders/instanced_shader.vs", "shaders/instanced_shader.fs") };
	Shader text_shader_{ Shader("shaders/text_shader.vs", "shaders/text_shader.fs") };
	Shader post_shader_{ Shader("shaders/post_shader.vs", "shaders/post_shader.fs") };
	Shader particle_shader_{ Shader("shaders/particle_shader.vs", "shaders/particle_shader.gs", "shaders/particle_shader.fs") };

private:
	GLFWwindow* window_;

	GLuint atlas_;

	GLuint fbo_;
	GLuint color_tex_;
	GLuint rbo_;
	GLuint screen_vao_;
	GLuint screen_vbo_;

	GraphicsState state_ = GraphicsState::None;
	int state_counter_ = 0;

	void load_texture_atlas();
};

class TextRenderer {
public:
	TextRenderer(FontManager* fonts);
	~TextRenderer();

	void draw();
	void update_drawers();
	void reset();
	void draw_ui();
	void draw_text();

	void toggle_string_drawer(StringDrawer* drawer, bool active);

	FontManager* fonts_;
	std::vector<ProtectedStringDrawer> string_drawers_{};

	GLuint ui_atlas_;

private:
	Shader ui_shader_{ Shader("shaders/ui_shader.vs", "shaders/ui_shader.fs") };
	Shader* text_shader_{};
};

#endif // GRAPHICSMANAGER_H
