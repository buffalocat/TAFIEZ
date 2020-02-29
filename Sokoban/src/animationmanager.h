#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <array>
#include <random>

#include "delta.h"

enum class Direction;
enum class ParticleTexture;
class GameObject;
class SoundManager;
class ClearFlag;

enum class AnimationSignal {
	NONE,
	IncineratorOn,
	IncineratorOff,
	IncineratorBurn,
	DoorOn,
	DoorOff,
	SwitchOn,
	SwitchOff,
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


class ParticleSource {
public:
	ParticleSource();
	virtual ~ParticleSource();
	virtual bool update(RandDouble& rand, ParticleVector& particles) = 0;
};

using SourceMap = std::map<GameObject*, ParticleSource*>;


class FireParticle : public Particle {
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


class EmberSource : public ParticleSource {
public:
	EmberSource(GameObject* parent, bool active);
	virtual ~EmberSource();
	bool update(RandDouble& rand, ParticleVector& particles);

	GameObject* parent_;
	bool active_;
};


class FlameSource : public ParticleSource {
public:
	FlameSource(GameObject* parent);
	virtual ~FlameSource();
	bool update(RandDouble& rand, ParticleVector& particles);
	
	GameObject* parent_;
	int life_;
};


class DoorVortexParticle : public Particle {
public:
	DoorVortexParticle(glm::vec3 center, RandDouble& rand);
	virtual ~DoorVortexParticle();
	void get_vertex(std::vector<ParticleVertex>&);
	bool update();

private:
	glm::vec3 c_;
	double dz_;
	double rad_;
	double theta_;
	int life_;
};


class DoorVortexSource : public ParticleSource {
public:
	DoorVortexSource(GameObject* parent, bool active);
	virtual ~DoorVortexSource();
	bool update(RandDouble& rand, ParticleVector& particles);

	GameObject* parent_;
	bool active_;
};


class FlagSparkle : public Particle {
public:
	FlagSparkle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color);
	~FlagSparkle();
	bool update();
	void get_vertex(std::vector<ParticleVertex>&);

private:
	glm::vec3 pos_, vel_;
	glm::vec3 color_;
	int life_;
	bool fading_;
};

class FlagSparkleSource : public ParticleSource {
public:
	FlagSparkleSource(ClearFlag* flag);
	~FlagSparkleSource();
	bool update(RandDouble& rand, ParticleVector& particles);

private:
	glm::vec3 pos_;
	glm::vec3 color_;
};


class SnakeSplitParticle : public Particle {
public:
	SnakeSplitParticle(glm::vec3 center, glm::vec4 color, RandDouble& rand);
	~SnakeSplitParticle();
	void get_vertex(std::vector<ParticleVertex>&);
	bool update();

private:
	glm::vec3 pos_, vel_;
	glm::vec4 color_;
	float size_;
	int life_;
};


class SnakeSplitSource : public ParticleSource {
public:
	SnakeSplitSource(GameObject* obj);
	~SnakeSplitSource();
	bool update(RandDouble& rand, ParticleVector& particles);

private:
	glm::vec3 pos_;
	glm::vec4 color_;
};


class AnimationManager {
public:
	AnimationManager(Shader* shader);
	~AnimationManager();

	void update();
	void abort_move();
	void reset_particles();
	void render_particles(glm::vec3 view_dir);
	void create_bound_source(GameObject* obj, std::unique_ptr<ParticleSource> source);

	
	void set_linear_animation(Direction dir, GameObject* obj);
	void set_linear_animation_frames();
	void make_fall_trail(GameObject* obj, int height);

	void receive_signal(AnimationSignal signal, GameObject* obj, DeltaFrame*);

private:
	// "Keyed" on Direction (minus 1)
	std::array<std::vector<GameObject*>, 6> linear_animations_{};
	std::vector<std::unique_ptr<ParticleSource>> sources_{};
	std::vector<std::unique_ptr<Particle>> particles_{};
	std::unique_ptr<SoundManager> sounds_{};

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