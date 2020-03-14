#include "stdafx.h"
#include "animationmanager.h"

#include <stb_image.h>

#include "playingstate.h"
#include "graphicsmanager.h"
#include "gameobject.h"
#include "common_constants.h"
#include "texture_constants.h"
#include "color_constants.h"
#include "soundmanager.h"

#include "car.h"
#include "clearflag.h"
#include "door.h"
#include "flaggate.h"
#include "incinerator.h"
#include "gate.h"

Particle::Particle() {}

Particle::~Particle() {}


const int FIRE_PART_MAX_LIFE = 60;

FireParticle::FireParticle(glm::vec3 c, ParticleTexture type, double range, double size, RandDouble& rand) : Particle() {
	pos_ = glm::vec3(c.x + range * (rand() - 0.5), c.y + range * (rand() - 0.5), c.z + 0.5);
	vel_ = glm::vec3(0.01 * (rand() - 0.5), 0.01 * (rand() - 0.5), 0.01 + 0.004 * rand());
	tex_ = tex_to_vec(type);
	size_ = glm::vec2(size, size);
	life_ = FIRE_PART_MAX_LIFE;
}

FireParticle::~FireParticle() {}

void FireParticle::get_vertex(std::vector<ParticleVertex>& vertices) {
	float rl = life_ / (float)FIRE_PART_MAX_LIFE;
	glm::vec4 color = glm::vec4(
		1.0,
		0.6 * rl,
		0.3 * rl,
		rl);
	vertices.push_back(ParticleVertex{
		pos_,
		tex_,
		size_,
		color });
}

bool FireParticle::update() {
	pos_ += vel_;
	return --life_ == 0;
}


ParticleSource::ParticleSource() {}

ParticleSource::~ParticleSource() {}


EmberSource::EmberSource(GameObject* parent) : ParticleSource(),
parent_{ parent } {}

EmberSource::~EmberSource() {}

bool EmberSource::update(RandDouble& rand, ParticleVector& particles) {
	if (rand() > 0.8) {
		particles.push_back(std::make_unique<FireParticle>(glm::vec3(parent_->real_pos()), ParticleTexture::SolidSquare, 0.8, 0.05, rand));
	}
	return !active_;
}


const int FLAME_SOURCE_MAX_LIFE = 8;

FlameSource::FlameSource(GameObject* parent) : ParticleSource(),
parent_{ parent }, life_{ FLAME_SOURCE_MAX_LIFE } {}

FlameSource::~FlameSource() {}

bool FlameSource::update(RandDouble& rand, ParticleVector& particles) {
	FPoint3 p = parent_->real_pos();
	for (int i = 0; i < 3; ++i) {
		glm::vec3 shifted_center = glm::vec3(p.x, p.y, p.z + rand());
		particles.push_back(std::make_unique<FireParticle>(shifted_center, ParticleTexture::SolidSquare, 1.0, 0.15 + 0.1 * rand(), rand));
	}
	return --life_ <= 0;
}


const double DOOR_PART_RAD_RATE = 0.01;
const double DOOR_PART_Z_RATE = 0.002;
const double DOOR_PART_THETA_RATE = 0.02;
const int DOOR_PART_MAX_LIFE = 30;

DoorVortexParticle::DoorVortexParticle(glm::vec3 c, RandDouble& rand) : Particle(),
c_{ c }	{
	rad_ = 0.7;
	dz_ = 0.6;
	theta_ = rand() * TWO_PI;
	life_ = 0;
}

DoorVortexParticle::~DoorVortexParticle() {}

void DoorVortexParticle::get_vertex(std::vector<ParticleVertex>& vertices) {
	vertices.push_back(ParticleVertex{
		glm::vec3(c_.x + rad_ * cos(theta_), c_.y + rad_ * sin(theta_), c_.z + dz_),
		tex_to_vec(ParticleTexture::SolidSquare),
		glm::vec2(0.05f),
		glm::vec4(0.8f, 0.5f, 0.9f, float(life_) / DOOR_PART_MAX_LIFE) });
}

bool DoorVortexParticle::update() {
	rad_ -= DOOR_PART_RAD_RATE;
	dz_ -= DOOR_PART_Z_RATE;
	theta_ += DOOR_PART_THETA_RATE;
	++life_;
	return rad_ <= 0.3 || dz_ <= 0.5 || life_ >= DOOR_PART_MAX_LIFE;
}


DoorVortexSource::DoorVortexSource(GameObject* parent) : ParticleSource(),
parent_{ parent } {}

DoorVortexSource::~DoorVortexSource() {}

bool DoorVortexSource::update(RandDouble& rand, ParticleVector& particles) {
	if (rand() > 0.80) {
		particles.push_back(std::make_unique<DoorVortexParticle>(glm::vec3(parent_->real_pos()), rand));
	}
	return !active_;
}


const int FLAG_SPARKLE_MAX_LIFE = 40;

FlagSparkle::FlagSparkle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color) : Particle(),
pos_{ pos }, vel_{ vel }, color_{ color }, life_{ 0 }, fading_{ false } {}

FlagSparkle::~FlagSparkle() {}

bool FlagSparkle::update() {
	if (fading_) {
		--life_;
	} else {
		++life_;
		fading_ = (life_ == FLAG_SPARKLE_MAX_LIFE);
	}
	pos_ += vel_;
	return life_ == 0;
}

void FlagSparkle::get_vertex(std::vector<ParticleVertex>& vertices) {
	vertices.push_back(ParticleVertex{
		pos_,
		tex_to_vec(ParticleTexture::Diamond),
		glm::vec2(0.10f),
		glm::vec4(color_, float(life_) / FLAG_SPARKLE_MAX_LIFE) });
}

FlagSparkleSource::FlagSparkleSource(ClearFlag* flag): ParticleSource(),
	flag_{ flag } {}

FlagSparkleSource::~FlagSparkleSource() {}

bool FlagSparkleSource::update(RandDouble& rand, ParticleVector& particles) {
	if (rand() > 0.95) {
		glm::vec3 d(rand() - 0.5f, rand() - 0.5f, rand() - 0.5f);
		glm::vec4 c = COLOR_VECTORS[flag_->color()];
		glm::vec3 color = glm::vec3(c.x, c.y, c.z);
		particles.push_back(std::make_unique<FlagSparkle>(
			glm::vec3(flag_->parent_->real_pos()) + glm::vec3(0.6f) * d + glm::vec3(0,0,1.2f),
			glm::vec3(0.01f) * d,
			glm::vec3(0.8f) * color));
	}
	return false;
}


const int SNAKE_SPLIT_PART_MAX_LIFE = 36;

SnakeSplitParticle::SnakeSplitParticle(glm::vec3 center, glm::vec3 color, RandDouble& rand): Particle() {
	color_ = glm::vec3((float)(0.5 + 0.4 * rand())) * color;
	glm::vec3 dir = glm::vec3(rand() - 0.5f, rand() - 0.5f, rand() - 0.5f);
	pos_ = center + glm::vec3(0.5f) * dir;
	vel_ = glm::vec3(0.04f) * dir;
	size_ = (float)(0.15 + 0.25 * rand());
	life_ = 30 + (int)(6 * rand());
}

SnakeSplitParticle::~SnakeSplitParticle() {}

void SnakeSplitParticle::get_vertex(std::vector<ParticleVertex>& vertices) {
	vertices.push_back(ParticleVertex{
		pos_,
		tex_to_vec(ParticleTexture::Diamond),
		glm::vec2(size_),
		glm::vec4(color_, (float)life_ / (float)SNAKE_SPLIT_PART_MAX_LIFE) });
}

bool SnakeSplitParticle::update() {
	pos_ += vel_;
	--life_;
	return life_ <= 0;
}


SnakeSplitSource::SnakeSplitSource(GameObject* obj) {
	pos_ = glm::vec3(obj->real_pos());
	glm::vec4 c = COLOR_VECTORS[obj->color()];
	color_ = glm::vec3(c.x, c.y, c.z);
}

SnakeSplitSource::~SnakeSplitSource() {}

bool SnakeSplitSource::update(RandDouble& rand, ParticleVector& particles) {
	for (int i = 0; i < 20; ++i) {
		particles.push_back(std::make_unique<SnakeSplitParticle>(pos_, color_, rand));
	}
	return true;
}


const int COLOR_SHELL_PART_MAX_LIFE = 16;


ColorShellParticle::ColorShellParticle(glm::vec3 center, glm::vec3 color, RandDouble& rand) :
	Particle(), color_{ color } {
	color_ = glm::vec3((float)(0.5 + 0.4 * rand())) * color;
	glm::vec3 dir = glm::vec3(rand() - 0.5f, rand() - 0.5f, rand() - 0.5f);
	float len = glm::length(dir);
	if (len > 0.0) {
		dir = dir * glm::vec3(1.0f / len);
	}
	pos_ = center + glm::vec3(0.7f) * dir;
	vel_ = glm::vec3(0.01f) * dir;
	life_ = COLOR_SHELL_PART_MAX_LIFE;
}

ColorShellParticle::~ColorShellParticle() {}

void ColorShellParticle::get_vertex(std::vector<ParticleVertex>& vertices) {
	vertices.push_back(ParticleVertex{
		pos_,
		tex_to_vec(ParticleTexture::SolidSquare),
		glm::vec2(0.1f),
		glm::vec4(color_, (float)life_ / (float)COLOR_SHELL_PART_MAX_LIFE) });
}

bool ColorShellParticle::update() {
	pos_ += vel_;
	--life_;
	return life_ <= 0;
}


ColorShellSource::ColorShellSource(GameObject* obj) {
	pos_ = glm::vec3(obj->real_pos());
	glm::vec4 c = COLOR_VECTORS[obj->color()];
	color_ = glm::vec3(c.x, c.y, c.z);
}

ColorShellSource::~ColorShellSource() {}

bool ColorShellSource::update(RandDouble& rand, ParticleVector& particles) {
	for (int i = 0; i < 50; ++i) {
		particles.push_back(std::make_unique<ColorShellParticle>(pos_, color_, rand));
	}
	return true;
}


RandDouble::RandDouble() {
	std::random_device rd;
	engine_ = std::mt19937_64(rd());
	dist_ = std::uniform_real_distribution<double>(0.0, 1.0);
}

RandDouble::~RandDouble() {}

double RandDouble::operator()() {
	return dist_(engine_);
}


AnimationManager::AnimationManager(Shader* shader, PlayingState* state, GLuint particle_atlas) :
	particle_shader_{ shader }, sounds_{ std::make_unique<SoundManager>() },
	state_{ state }, gfx_{ state->gfx_ }, particle_atlas_{ particle_atlas } {
	initialize_particle_shader();
}

AnimationManager::~AnimationManager() {
	glDeleteVertexArrays(1, &particle_VAO_);
	glDeleteBuffers(1, &particle_VBO_);
	// Force destruction of sources while source_map_ is valid
	sources_.clear();
}

void AnimationManager::initialize_particle_shader() {
	particle_shader_->use();
	particle_shader_->setFloat("texScale", 1.0f / PARTICLE_TEXTURE_ATLAS_SIZE);
	particle_shader_->setFloat("aspectRatio", (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT);
	glGenVertexArrays(1, &particle_VAO_);
	glBindVertexArray(particle_VAO_);
	glGenBuffers(1, &particle_VBO_);
	glBindBuffer(GL_ARRAY_BUFFER, particle_VBO_);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, TexCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, Size));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, Color));
}

const int MAX_DOOR_SQUISH_FRAMES = 8;

void AnimationManager::update() {
	// Update linear animation
	if (linear_animation_frames_ > 0) {
		--linear_animation_frames_;
		for (int i = 0; i < 6; ++i) {
			FPoint3 dpos = (-(float)(linear_animation_frames_) / HORIZONTAL_MOVEMENT_FRAMES) * FPoint3 { DIRECTIONS[i] };
			for (auto* obj : linear_animations_[i]) {
				obj->dpos_ = dpos;
			}
		}
		if (linear_animation_frames_ == 0) {
			for (int i = 0; i < 6; ++i) {
				linear_animations_[i].clear();
			}
		}
	}
	// Update door squishing
	if (door_squish_frames_ > 0) {
		--door_squish_frames_;
		for (auto& p : door_entering_objects_) {
			p.obj->draw_squished(gfx_, p.pos, (float)door_squish_frames_ / (float)MAX_DOOR_SQUISH_FRAMES);
		}
		if (door_squish_frames_ == 0) {
			door_entering_objects_.clear();
		}
	}
	// Update other animation
	for (auto* mod : static_animated_objects_) {
		mod->update_animation(state_);
	}
	temp_animated_objects_.erase(std::remove_if(temp_animated_objects_.begin(), temp_animated_objects_.end(),
		[this](auto& p) { return p->update_animation(state_); }), temp_animated_objects_.end());
	// Update fall trails
	for (auto& trail : fall_trails_) {
		--trail.opacity;
	}
	fall_trails_.erase(std::remove_if(fall_trails_.begin(), fall_trails_.end(),
		[](FallTrail t) {return t.opacity == 0; }), fall_trails_.end());
	// Update particles and remove dead ones
	sources_.erase(std::remove_if(sources_.begin(), sources_.end(),
		[this](auto& p) { return p->update(rand_, particles_); }), sources_.end());
	particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
		[](auto& p) { return p->update(); }), particles_.end());
	// Run sound engine
	sounds_->flush_sounds();
}

void AnimationManager::reset_temp() {
	// Cancel linear animation
	for (int i = 0; i < 6; ++i) {
		for (auto* obj : linear_animations_[i]) {
			obj->dpos_ = {};
		}
	}
	// Cancel any particles/text that shouldn't exist...?
	for (auto* mod : temp_animated_objects_) {
		mod->reset_animation();
	}
	temp_animated_objects_.clear();
	fall_trails_.clear();
	door_entering_objects_.clear();
}

void AnimationManager::reset() {
	reset_temp();
	for (auto* mod : static_animated_objects_) {
		mod->reset_animation();
	}
	static_animated_objects_.clear();
	sources_.clear();
	particles_.clear();
	source_map_.clear();
}

void AnimationManager::create_bound_source(GameObject* obj, std::unique_ptr<ParticleSource> source) {
	source_map_[obj] = source.get();
	sources_.push_back(std::move(source));
}

const unsigned int FALL_TRAIL_OPACITY = 8;
const double FALL_TRAIL_MAX_OPACITY = 10.0;
const double FALL_TRAIL_MAX_WIDTH = 16.0;

void AnimationManager::make_fall_trail(GameObject* block, int height, int drop) {
	fall_trails_.push_back({ block->pos_ + Point3{0,0,height}, height + drop, FALL_TRAIL_OPACITY, block->color() });
}

void AnimationManager::draw_fall_trails() {
	std::sort(fall_trails_.begin(), fall_trails_.end(), [this](FallTrail a, FallTrail b) {
		return glm::dot(glm::vec3(a.base - b.base), view_dir_) < 0; });
	for (auto& trail : fall_trails_) {
		glm::vec4 color = COLOR_VECTORS[trail.color];
		color.w = (float)(trail.opacity / FALL_TRAIL_MAX_OPACITY);
		Point3 base = trail.base;
		gfx_->cube.push_instance(glm::vec3(base.x, base.y, base.z + 0.5f - trail.height / 2.0f),
			glm::vec3(trail.opacity / FALL_TRAIL_MAX_WIDTH, trail.opacity / FALL_TRAIL_MAX_WIDTH, trail.height),
			BlockTexture::Blank, color);
	}
	gfx_->cube.draw();
}

void AnimationManager::render_particles() {
	std::vector<ParticleVertex> vertices{};
	for (auto& particle : particles_) {
		particle->get_vertex(vertices);
	}
	std::sort(vertices.begin(), vertices.end(), [this](ParticleVertex a, ParticleVertex b) {
		return glm::dot((a.Position - b.Position), view_dir_) < 0; });
	glBindTexture(GL_TEXTURE_2D, particle_atlas_);
	glBindVertexArray(particle_VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, particle_VBO_);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ParticleVertex), vertices.data(), GL_STATIC_DRAW);
	glDrawArrays(GL_POINTS, 0, (GLsizei)vertices.size());
}

void AnimationManager::set_linear_animation(Direction dir, GameObject* obj) {
	linear_animations_[static_cast<int>(dir) - 1].push_back(obj);
}

void AnimationManager::set_linear_animation_frames() {
	linear_animation_frames_ = HORIZONTAL_MOVEMENT_FRAMES;
}

void AnimationManager::receive_signal(AnimationSignal signal, GameObject* obj, DeltaFrame* delta_frame) {
	switch (signal) {
	case AnimationSignal::IncineratorOn:
	{
		auto* inc = static_cast<Incinerator*>(obj->modifier());
		if (!source_map_.count(obj)) {
			create_bound_source(obj, std::make_unique<EmberSource>(obj));
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::IncineratorOff, obj));
			}
		}
		break;
	}
	case AnimationSignal::IncineratorOff:
	{
		if (source_map_.count(obj)) {
			auto* source = static_cast<EmberSource*>(source_map_[obj]);
			source->active_ = false;
			source_map_.erase(obj);
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::IncineratorOn, obj));
			}
		}
		break;
	}
	case AnimationSignal::IncineratorBurn:
		sources_.push_back(std::make_unique<FlameSource>(obj));
		break;
	case AnimationSignal::DoorOn:
	{
		auto* door = static_cast<Door*>(obj->modifier());
		if (!source_map_.count(obj)) {
			create_bound_source(obj, std::make_unique<DoorVortexSource>(obj));
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::DoorOff, obj));
			}
		}
		break;
	}
	case AnimationSignal::DoorOff:
	{
		if (source_map_.count(obj)) {
			auto* source = static_cast<DoorVortexSource*>(source_map_[obj]);
			source->active_ = false;
			source_map_.erase(obj);
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::DoorOn, obj));
			}
		}
		break;
	}
	case AnimationSignal::SwitchOn:
		//sounds_->queue_sound(SoundName::SwitchOn);
		break;
	case AnimationSignal::SwitchOff:
		//sounds_->queue_sound(SoundName::SwitchOff);
		break;
	case AnimationSignal::SnakeSplit:
		sounds_->queue_sound(SoundName::SnakeSplit);
		sources_.push_back(std::make_unique<SnakeSplitSource>(obj));
		break;
	case AnimationSignal::ColorChange:
		sources_.push_back(std::make_unique<ColorShellSource>(obj));
		break;
	case AnimationSignal::FlagOn:
	{
		auto* flag = static_cast<ClearFlag*>(obj->modifier());
		if (!source_map_.count(obj)) {
			create_bound_source(obj, std::make_unique<FlagSparkleSource>(flag));
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::FlagOff, obj));
			}
		}
		break;
	}
	case AnimationSignal::FlagOff:
	{
		if (source_map_.count(obj)) {
			auto* source = static_cast<FlagSparkleSource*>(source_map_[obj]);
			source->active_ = false;
			source_map_.erase(obj);
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::FlagOn, obj));
			}
		}
		break;
	}
	case AnimationSignal::FlagGateOn:
	{
		auto* flag_gate = obj->modifier();
		if (!static_animated_objects_.count(flag_gate)) {
			static_animated_objects_.insert(flag_gate);
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::FlagGateOff, obj));
			}
		}
		break;
	}
	case AnimationSignal::FlagGateOff:
	{
		auto* flag_gate = obj->modifier();
		if (static_animated_objects_.count(flag_gate)) {

			static_animated_objects_.erase(flag_gate);
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::FlagGateOn, obj));
			}
		}
		break;
	}
	case AnimationSignal::GateUp:
	{
		auto* gate = static_cast<Gate*>(obj->modifier());
		gate->start_raise_animation();
		temp_animated_objects_.push_back(gate);
		break;
	}
	case AnimationSignal::GateDown:
	{
		auto* gate = static_cast<Gate*>(obj->modifier());
		gate->start_lower_animation();
		temp_animated_objects_.push_back(gate);
		break;
	}
	case AnimationSignal::CarRide:
	{
		auto* car = static_cast<Car*>(obj->modifier());
		temp_animated_objects_.push_back(car);
		car->animation_state_ = CarAnimationState::Riding;
		car->animation_time_ = MAX_CAR_ANIMATION_FRAMES;
		break;
	}
	case AnimationSignal::CarUnride:
	{
		auto* car = static_cast<Car*>(obj->modifier());
		temp_animated_objects_.push_back(car);
		car->animation_state_ = CarAnimationState::Unriding;
		car->animation_time_ = MAX_CAR_ANIMATION_FRAMES;
		break;
	}
	case AnimationSignal::DoorEnter:
	{
		door_entering_objects_.push_back({ obj, obj->real_pos() });
		door_squish_frames_ = MAX_DOOR_SQUISH_FRAMES;
	}
	case AnimationSignal::DoorExit:
	{

	}
	}
}

AnimationSignalDelta::AnimationSignalDelta(AnimationManager* anims, AnimationSignal signal, GameObject* obj) :
	Delta(), anims_{ anims }, signal_{ signal }, obj_{ obj } {}

AnimationSignalDelta::~AnimationSignalDelta() {}

void AnimationSignalDelta::revert() {
	anims_->receive_signal(signal_, obj_, nullptr);
}