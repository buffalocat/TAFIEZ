#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "color_constants.h"
#include "modelinstancer.h"

struct GLFWwindow;
class FontManager;
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

class GraphicsManager {
public:
    GraphicsManager(GLFWwindow*);
	~GraphicsManager();

    GLFWwindow* window();

	void prepare_object_rendering();
    void set_PV(glm::mat4);
	void set_light_source(glm::vec3);
    void draw();

	// These must be drawn (in a batch) in draw_world()
	DynamicInstancer cube{ DynamicInstancer("resources/uniform_cube.obj") };
	DynamicInstancer top_cube{ DynamicInstancer("resources/top_cube.obj") };
	DynamicInstancer six_squares{ DynamicInstancer("resources/six_squares.obj") };

	DynamicInstancer diamond{ DynamicInstancer("resources/diamond.obj") };
	DynamicInstancer top_diamond{ DynamicInstancer("resources/top_diamond.obj") };
	DynamicInstancer six_squares_diamond{ DynamicInstancer("resources/six_squares_diamond.obj") };

	DynamicInstancer cube_edges{ DynamicInstancer("resources/cube_edges.obj") };

	// Unused for now
	StaticInstancer wall{ StaticInstancer("resources/uniform_cube.obj") };

	// Models which aren't common enough to be worth instancing
	SingleDrawer windshield{ SingleDrawer("resources/windshield.obj") };
	SingleDrawer windshield_diamond{ SingleDrawer("resources/windshield_diamond.obj") };
	SingleDrawer flag{ SingleDrawer("resources/flag.obj") };

	std::unique_ptr<FontManager> fonts_{};

private:
	GLFWwindow* window_;
	std::vector<Animation*> animations;
	GLuint atlas_;
	Shader instanced_shader_{ Shader("shaders/instanced_shader.vs", "shaders/instanced_shader.fs") };
	Shader text_shader_{ Shader("shaders/text_shader.vs", "shaders/text_shader.fs") };

	glm::mat4 PV_;

    void load_texture_atlas();
};

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
