#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <sstream>

typedef char GLchar;

class Shader {
public:
    unsigned int ID;

    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
    ~Shader();
    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, double value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec2(const std::string& name, const glm::vec2& vec) const;
	void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setVec4(const std::string& name, const glm::vec4& vec) const;
};

#endif // SHADER_H
