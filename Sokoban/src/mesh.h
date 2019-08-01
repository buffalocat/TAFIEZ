#pragma once

struct Vertex;

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Mesh(std::vector<Vertex>&&, std::vector<unsigned int>&&);
	~Mesh();
	void draw(unsigned int instance_count);
private:
	unsigned int VAO, VBO, EBO;
	void setup_mesh();

	friend class Model;
};