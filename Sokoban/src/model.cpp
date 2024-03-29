#include "stdafx.h"
#include "model.h"

#include "mesh.h"
#include "vertexlayout.h"
#include "shader.h"

Model::Model(std::string const& path) : meshes{} {
	load_model(path);
}

Model::~Model() {}

void Model::load_model(std::string const& path) {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		LOG("ERROR::ASSIMP::" << importer.GetErrorString());
		return;
	}

	process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode* node, aiScene const* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		process_node(node->mChildren[i], scene);
	}
}

Mesh Model::process_mesh(aiMesh* mesh, aiScene const* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;
		vertex.Position.x = mesh->mVertices[i].x;
		vertex.Position.y = mesh->mVertices[i].y;
		vertex.Position.z = mesh->mVertices[i].z;
		vertex.Normal.x = mesh->mNormals[i].x;
		vertex.Normal.y = mesh->mNormals[i].y;
		vertex.Normal.z = mesh->mNormals[i].z;
		if (mesh->mTextureCoords[0]) {
			vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	return Mesh(std::move(vertices), std::move(indices));
}

std::vector<int> Model::gather_mesh_vao() {
	std::vector<int> vao_vec{};
	for (Mesh& mesh : meshes) {
		vao_vec.push_back(mesh.VAO);
	}
	return vao_vec;
}

void Model::draw_instanced(unsigned int instance_count) {
	for (Mesh& mesh : meshes) {
		mesh.draw_instanced(instance_count);
	}
}

void Model::draw_single() {
	for (Mesh& mesh : meshes) {
		mesh.draw_single();
	}
}