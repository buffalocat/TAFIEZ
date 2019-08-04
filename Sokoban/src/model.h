#pragma once

class Mesh;

class Model {
public: 
	Model(std::string const&);
	~Model();
	void draw_instanced(unsigned int instance_count);
	void draw_single();
	std::vector<int> gather_mesh_vao();
private:
	std::vector<Mesh> meshes;
	void load_model(std::string const&);
	void process_node(aiNode* node, aiScene const* scene);
	Mesh process_mesh(aiMesh* mesh, aiScene const* scene);
};