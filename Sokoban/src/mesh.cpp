#include "stdafx.h"

#include <glad/glad.h>

#include "mesh.h"
#include "vertexlayout.h"

Mesh::Mesh(std::vector<Vertex>&& vs, std::vector<unsigned int>&& is) : vertices{ vs }, indices{ is } {
	setup_mesh();
}

Mesh::~Mesh() {}

void Mesh::setup_mesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	glVertexAttribDivisor(0, 0);
	// normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glVertexAttribDivisor(1, 0);
	// tex coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glVertexAttribDivisor(2, 0);

	glBindVertexArray(0);
}

void Mesh::bind_instance_buffer(unsigned int inst_buffer) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, inst_buffer);
	// pos offset
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, PosOffset));
	glVertexAttribDivisor(3, 1);
	// scales
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, Scale));
	glVertexAttribDivisor(4, 1);
	// tex offset
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, TexOffset));
	glVertexAttribDivisor(5, 1);
	// color
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, Color));
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
}

void Mesh::draw(unsigned int instance_count) {
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0, instance_count);
	glBindVertexArray(0);
}