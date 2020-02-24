#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <array>
#include <random>

#include "delta.h"

enum class Direction;
enum class ParticleTexture;
class GameObject;

enum class AnimationSignal {
	NONE,
	IncineratorOn,
	IncineratorOff,
	IncineratorBurn,
	DoorOn,
	DoorOff,
	SwitchPress,
	SwitchRelease,
	GateUp,
	GateDown,
	LinkBreak,
	SnakeSplit,
	FlagExists,
	FlagActivate,
	FlagDeactivate,
	FlagCollect,
	DoorEnter,
	DoorExit,
	CarRide,
	CarUnride,
	ConvertibleRide,
	ConvertibleUnride,
	ColorBind,
	ColorUnbind,
	SignOn,
	SignOff,
};

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

using ParticleVector = std::vector<std::unique_ptr<Particle>>;

class FireParticle: public Particle {
public:
	FireParticle(glm::vec3 center, ParticleTexture type, double range, double size, RandDouble& rand);
	virtual ~FireParticle();

	void get_vertex(std::vector<ParticleVertex>&);
	bool update();
	
private:
	glm::vec3 pos_, vel_;
	glm::vec2 tex_, size_;
	int life_;
};


class ParticleSource {
public:
	ParticleSource();
	virtual ~ParticleSource();
	virtual bool update(RandDouble& rand, ParticleVector& particles) = 0;
};

using SourceMap = std::map<GameObject*, ParticleSource*>;

class EmberSource: public ParticleSource {
public:
	EmberSource(GameObject* parent, SourceMap& source_map);
	virtual ~EmberSource();
	bool update(RandDouble& rand, ParticleVector& particles);

	GameObject* parent_;
	SourceMap& source_map_;
	bool supported_ = true;
};


class FlameSource : public ParticleSource {
public:
	FlameSource(GameObject* parent);
	virtual ~FlameSource();
	bool update(RandDouble& rand, ParticleVector& particles);
	
	GameObject* parent_;
	int life_;
};


class AnimationManager {
public:
	AnimationManager(Shader* shader);
	~AnimationManager();

	void update();
	void abort_move();
	void reset_particles();
	void render_particles(glm::vec3 view_dir);
	
	void set_linear_animation(Direction dir, GameObject* obj);
	void set_linear_animation_frames();
	void make_fall_trail(GameObject* obj, int height);

	void receive_signal(AnimationSignal signal, GameObject* obj, DeltaFrame*);

private:
	// "Keyed" on Direction (minus 1)
	std::array<std::vector<GameObject*>, 6> linear_animations_{};
	std::vector<std::unique_ptr<ParticleSource>> sources_{};
	std::vector<std::unique_ptr<Particle>> particles_{};

	int linear_animation_frames_ = -1;

	SourceMap source_map_{};

	Shader* particle_shader_;
	GLuint particle_VAO_, particle_VBO_, atlas_;
	RandDouble rand_{};

	void initialize_particle_shader();
};

class AnimationSignalDelta : public Delta {
public:
	AnimationSignalDelta(AnimationManager* anims, AnimationSignal signal, GameObject* parent);
	~AnimationSignalDelta();

	void revert();

private:
	AnimationManager* anims_;
	AnimationSignal signal_;
	GameObject* obj_;
};

#endif ANIMATIONMANAGER_H