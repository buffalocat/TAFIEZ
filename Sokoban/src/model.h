#pragma once

#include <filesystem>
#include <string>

#include <assimp/scene.h>

class Mesh;
class Shader;

class Model {
public: 
	Model(std::string const&);
	~Model();
	void draw(Shader shader, unsigned int instance_count = 1);
	std::vector<int> gather_mesh_vao();
private:
	std::vector<Mesh> meshes;
	void load_model(std::string const&);
	void process_node(aiNode* node, aiScene const* scene);
	Mesh process_mesh(aiMesh* mesh, aiScene const* scene);
};