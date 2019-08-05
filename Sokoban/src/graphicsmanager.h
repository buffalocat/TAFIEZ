#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "color_constants.h"

#include "modelinstancer.h"

struct GLFWwindow;
class TextRenderer;

class GraphicsManager {
public:
    GraphicsManager(GLFWwindow*);
	~GraphicsManager();

    GLFWwindow* window();

	void setup_graphics();
    void set_PV(glm::mat4);
    void draw_world();

	void render_text(std::string text, float opacity);

	// These must be drawn (in a batch) in draw_world()
	DynamicInstancer cube;
	DynamicInstancer top_cube;
	DynamicInstancer diamond;
	DynamicInstancer six_squares;
	WallInstancer wall;

	// Models which aren't common enough to be worth instancing
	SingleDrawer windshield;
	SingleDrawer windshield_diamond;

private:
	unsigned int atlas_;
    GLFWwindow* window_;
	std::unique_ptr<TextRenderer> text_;
	Shader instanced_shader_;

	glm::mat4 PV_;

    void load_texture_atlas();
};

#endif // GRAPHICSMANAGER_H
