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
class GraphicsManager;
class ObjectModifier;
class PlayingState;
class PlayingGlobalData;
class MapDisplay;
class StringDrawer;

enum class AnimationSignal {
	NONE,
	// Modifier Updates
	IncineratorOn,
	IncineratorOff,
	DoorOn,
	DoorOff,
	SwitchOn,
	SwitchOff,
	GateUp,
	GateDown,
	FlagOn,
	FlagOff,
	FlagSwitchOn,
	FlagSwitchOff,
	FlagGateOn,
	FlagGateOff,
	SignOn,
	SignOff,
	MapDisplay,
	// Other Actions
	FlagCollect,
	ColorChange,
	Jump,
	CarRide,
	CarUnride,
	ColorBind,
	ColorUnbind,
	LinkBreak,
	SnakeSplit,
	DoorEnter,
	DoorExit,
	IncineratorBurn,
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
	EmberSource(GameObject* parent);
	virtual ~EmberSource();
	bool update(RandDouble& rand, ParticleVector& particles);

	GameObject* parent_;
	bool active_ = true;
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
	DoorVortexSource(GameObject* parent);
	virtual ~DoorVortexSource();
	bool update(RandDouble& rand, ParticleVector& particles);

	GameObject* parent_;
	bool active_ = true;
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

	ClearFlag* flag_;
	bool active_ = true;
};


class SnakeSplitParticle : public Particle {
public:
	SnakeSplitParticle(glm::vec3 center, glm::vec3 color, RandDouble& rand);
	~SnakeSplitParticle();
	void get_vertex(std::vector<ParticleVertex>&);
	bool update();

private:
	glm::vec3 pos_, vel_;
	glm::vec3 color_;
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
	glm::vec3 color_;
};


class ColorShellParticle : public Particle {
public:
	ColorShellParticle(glm::vec3 center, glm::vec3 color, RandDouble& rand);
	~ColorShellParticle();
	void get_vertex(std::vector<ParticleVertex>&);
	bool update();

private:
	glm::vec3 pos_, vel_;
	glm::vec3 color_;
	int life_;
};

class ColorShellSource : public ParticleSource {
public:
	ColorShellSource(GameObject* obj);
	~ColorShellSource();
	bool update(RandDouble& rand, ParticleVector& particles);

private:
	glm::vec3 pos_;
	glm::vec3 color_;
};


class Cutscene {
public:
	Cutscene(GraphicsManager* gfx);
	virtual ~Cutscene() = 0;
	virtual bool update() = 0;
	virtual void draw() = 0;

protected:
	GraphicsManager* gfx_;
};

class FlagCutscene : public Cutscene {
public:
	FlagCutscene(GraphicsManager* gfx, PlayingGlobalData* global, char zone);
	~FlagCutscene();
	bool update();
	void draw();

private:
	static const int TOTAL_TIME;
	static const int FADE_TIME;
	static const int WAIT_TIME;

	std::vector<std::unique_ptr<StringDrawer>> lines_{};
	int time_;
	char zone_;
};


struct FallTrail {
	Point3 base;
	int height;
	int opacity;
	int color;
};


struct DoorSquish {
	GameObject* obj;
	FPoint3 pos;
};


class AnimationManager {
public:
	AnimationManager(Shader* shader, PlayingState* state, GLuint particle_atlas);
	~AnimationManager();

	void update();
	void reset_temp();
	void reset();
	void draw_fall_trails();
	void render_particles();
	void draw_special();
	void draw_cutscene();

	void start_flag_cutscene(PlayingGlobalData* global, char zone);
	void create_bound_source(GameObject* obj, std::unique_ptr<ParticleSource> source);

	void set_linear_animation(Direction dir, GameObject* obj);
	void set_linear_animation_frames();
	void make_fall_trail(GameObject*, int height, int drop);

	void receive_signal(AnimationSignal signal, GameObject* obj, DeltaFrame*);

	glm::vec3 view_dir_{};

	std::unique_ptr<SoundManager> sounds_{};

private:
	// "Keyed" on Direction (minus 1)
	std::array<std::vector<GameObject*>, 6> linear_animations_{};
	std::set<ObjectModifier*> static_animated_objects_{};
	std::vector<ObjectModifier*> temp_animated_objects_{};
	std::vector<std::unique_ptr<ParticleSource>> sources_{};
	std::vector<std::unique_ptr<Particle>> particles_{};
	std::vector<FallTrail> fall_trails_{};
	std::vector<DoorSquish> door_entering_objects_{};
	std::unique_ptr<Cutscene> cutscene_{};
	PlayingState* state_;

	int linear_animation_frames_ = -1;
	int door_squish_frames_ = 0;

	MapDisplay* map_display_{};

	std::map<GameObject*, ParticleSource*> source_map_{};

	Shader* particle_shader_;
	GraphicsManager* gfx_;
	GLuint particle_VAO_, particle_VBO_, particle_atlas_;
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