#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "color_constants.h"
#include "shader.h"

#include "modelinstancer.h"

struct GLFWwindow;

class GraphicsManager {
public:
    GraphicsManager(GLFWwindow*);
    GLFWwindow* window();

    void set_PV(glm::mat4, glm::mat4);

    void draw();

	ModelInstancer cube;

private:
    GLFWwindow* window_;
    Shader shader_;
	Shader instanced_shader_;

	glm::mat4 PV_;

    void load_texture_atlas();
};

#endif // GRAPHICSMANAGER_H
