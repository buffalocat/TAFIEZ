#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <random>

class GameObject;

class RandDouble {
public:
	RandDouble();
	~RandDouble();
	double operator()();

private:
	std::mt19937_64 engine_;
	std::uniform_real_distribution<double> dist_;
};


struct ParticleVertex {
	glm::vec3 Position;
	glm::vec2 TexCoords;
	glm::vec2 Size;
	glm::vec4 Color;
};

class Particle {
public:
	Particle();
	virtual ~Particle() = 0;

	virtual void get_vertex(std::vector<ParticleVertex>&) = 0;
	virtual bool update() = 0; // Return whether the particle has died
};

class FireParticle: public Particle {
public:
	FireParticle(glm::vec3 center, RandDouble& rand);
	virtual ~FireParticle();

	void get_vertex(std::vector<ParticleVertex>&);
	bool update();
	
private:
	glm::vec3 pos_, vel_;
	int life_;
};


class ParticleSource {
public:
	ParticleSource();
	~ParticleSource();

	virtual void get_vertices(std::vector<ParticleVertex>&) = 0;
	virtual bool update(RandDouble& rand) = 0;
};

class FireParticleSource: public ParticleSource {
public:
	FireParticleSource(Point3 center);
	virtual ~FireParticleSource();

	void get_vertices(std::vector<ParticleVertex>&);
	bool update(RandDouble& rand);

private:
	glm::vec3 center_;
	std::vector<std::unique_ptr<FireParticle>> particles_;
	int life_ = 60;
};


class AnimationManager {
public:
	AnimationManager(Shader* shader);
	~AnimationManager();

	void update();
	void render_particles(glm::vec3 view_dir);

private:
	std::vector<GameObject*> moving_objects_{};
	std::vector<std::unique_ptr<ParticleSource>> particle_sources_{};

	Shader* particle_shader_;
	GLuint particle_VAO_, particle_VBO_, atlas_;
	RandDouble rand_{};

	std::vector<ParticleVertex> particles_{};
};


#endif ANIMATIONMANAGER_H