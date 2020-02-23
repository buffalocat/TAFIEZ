#ifndef SHADER_H
#define SHADER_H

typedef char GLchar;

class Shader {
public:
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	Shader(const GLchar* vertex_path, const GLchar* geometry_path, const GLchar* fragment_path);
    ~Shader();
    void use();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, double value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec2(const std::string& name, const glm::vec2& vec) const;
	void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setVec4(const std::string& name, const glm::vec4& vec) const;

private:
	GLuint id_;
	GLuint compile_shader(const GLchar*, GLenum shader_type);
};

#endif // SHADER_H
