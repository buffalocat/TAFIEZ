#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "color_constants.h"
#include "modelinstancer.h"

struct GLFWwindow;
class TextRenderer;
class StringDrawer;
class Animation;

class GraphicsManager {
public:
    GraphicsManager(GLFWwindow*);
	~GraphicsManager();

    GLFWwindow* window();

	void prepare_object_rendering();
    void set_PV(glm::mat4);
	void set_light_source(glm::vec3);
    void draw_world();
	void draw_text();

	// These must be drawn (in a batch) in draw_world()
	DynamicInstancer cube;
	DynamicInstancer top_cube;
	DynamicInstancer diamond;
	DynamicInstancer six_squares;
	WallInstancer wall;

	// Models which aren't common enough to be worth instancing
	SingleDrawer windshield;
	SingleDrawer windshield_diamond;
	SingleDrawer flag;

	std::unique_ptr<TextRenderer> text_{ std::make_unique<TextRenderer>() };

private:
	GLFWwindow* window_;
	std::vector<Animation*> animations;
	GLuint atlas_;
	Shader instanced_shader_;

	glm::mat4 PV_;

    void load_texture_atlas();
};

#endif // GRAPHICSMANAGER_H
