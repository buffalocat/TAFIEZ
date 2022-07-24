#include "stdafx.h"
#include "shader.h"

Shader::Shader(const GLchar* vertex_path, const GLchar* fragment_path): id_ {0} {
    // Load in code for shaders
	GLuint vertex = compile_shader(vertex_path, GL_VERTEX_SHADER);
	GLuint fragment = compile_shader(fragment_path, GL_FRAGMENT_SHADER);
    // Compile shader program
	int success;
	char info_log[512];
    id_ = glCreateProgram();
    glAttachShader(id_, vertex);
    glAttachShader(id_, fragment);
    glLinkProgram(id_);
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id_, 512, nullptr, info_log);
		LOG("ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log);
    }
    // Clean up the shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::Shader(const GLchar* vertex_path, const GLchar* geometry_path, const GLchar* fragment_path) : id_{ 0 } {
	// Load in code for shaders
	GLuint vertex = compile_shader(vertex_path, GL_VERTEX_SHADER);
	GLuint geometry = compile_shader(geometry_path, GL_GEOMETRY_SHADER);
	GLuint fragment = compile_shader(fragment_path, GL_FRAGMENT_SHADER);
	// Compile shader program
	int success;
	char info_log[512];
	id_ = glCreateProgram();
	glAttachShader(id_, vertex);
	glAttachShader(id_, geometry);
	glAttachShader(id_, fragment);
	glLinkProgram(id_);
	glGetProgramiv(id_, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(id_, 512, nullptr, info_log);
		LOG("ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log);
	}
	// Clean up the shaders
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::~Shader() {}


GLuint Shader::compile_shader(const GLchar* path, GLenum shader_type) {
	std::string code;
	std::ifstream shader_file;
	shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		shader_file.open(path);
		std::stringstream shader_stream;
		shader_stream << shader_file.rdbuf();
		shader_file.close();
		code = shader_stream.str();
	} catch (std::ifstream::failure e) {
		LOG("Failed to read shader file.");
	}
	const char* code_raw = code.c_str();
	// Compile shaders
	int success;
	char info_log[512];

	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &code_raw, nullptr);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, info_log);
		LOG("Shader compilation failed.\n" << info_log);
	}
	return shader;

}

void Shader::use() {
    glUseProgram(id_);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(id_, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, double value) const {
    glUniform1f(glGetUniformLocation(id_, name.c_str()), (GLfloat)value);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &vec) const {
    glUniform2f(glGetUniformLocation(id_, name.c_str()), vec.x, vec.y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const {
	glUniform3f(glGetUniformLocation(id_, name.c_str()), vec.x, vec.y, vec.z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &vec) const {
    glUniform4f(glGetUniformLocation(id_, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}
