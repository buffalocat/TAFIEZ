#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "color_constants.h"
#include "modelinstancer.h"

struct GLFWwindow;
class FontManager;
class AnimationManager;
class Animation;
class StringDrawer;

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

	void pre_rendering();
	void draw_objects();
	void draw_particles();
	void post_rendering();

	DynamicInstancer cube{ "resources/uniform_cube.obj" };
	DynamicInstancer top_cube{ "resources/top_cube.obj" };
	DynamicInstancer six_squares{ "resources/six_squares.obj" };
	DynamicInstancer windshield{ "resources/windshield.obj" };

	DynamicInstancer diamond{ "resources/diamond.obj" };
	DynamicInstancer top_diamond{ "resources/top_diamond.obj" };
	DynamicInstancer six_squares_diamond{ "resources/six_squares_diamond.obj" };
	DynamicInstancer windshield_diamond{ "resources/windshield_diamond.obj" };

	DynamicInstancer cube_edges{ "resources/cube_edges.obj" };
	DynamicInstancer flag{ "resources/flag.obj" };
	
	DynamicInstancer square_0{ "resources/square_0.obj" };
	DynamicInstancer square_1{ "resources/square_1.obj" };
	DynamicInstancer square_2{ "resources/square_2.obj" };
	DynamicInstancer square_3{ "resources/square_3.obj" };

	std::unique_ptr<FontManager> fonts_{};
	std::unique_ptr<AnimationManager> anims_{};

	glm::mat4 proj_, view_;
	glm::vec3 light_source_;
	double shadow_;

private:
	GLFWwindow* window_;
	std::vector<Animation*> animations;

	GLuint atlas_;

	GLuint fbo_;
	GLuint color_tex_;
	GLuint rbo_;
	GLuint screen_vao_;
	GLuint screen_vbo_;

	GraphicsState state_ = GraphicsState::None;
	int state_counter_ = 0;

	Shader instanced_shader_{ Shader("shaders/instanced_shader.vs", "shaders/instanced_shader.fs") };	
	Shader text_shader_{ Shader("shaders/text_shader.vs", "shaders/text_shader.fs") };
	Shader post_shader_{ Shader("shaders/post_shader.vs", "shaders/post_shader.fs") };
	Shader particle_shader_{ Shader("shaders/particle_shader.vs", "shaders/particle_shader.gs", "shaders/particle_shader.fs") };

	void load_texture_atlas();
};

void prepare_text_rendering(Shader* text_shader);

class TextRenderer {
public:
	TextRenderer(FontManager* fonts);
	~TextRenderer();

	void draw();

	void toggle_string_drawer(StringDrawer* drawer, bool active);

	FontManager* fonts_;
	std::vector<ProtectedStringDrawer> string_drawers_{};

private:
	Shader* text_shader_{};
};

#endif // GRAPHICSMANAGER_H
