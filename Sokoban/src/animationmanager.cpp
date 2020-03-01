#include "stdafx.h"
#include "animationmanager.h"

#include <stb_image.h>

#include "gameobject.h"
#include "common_constants.h"
#include "texture_constants.h"
#include "color_constants.h"
#include "soundmanager.h"
#include "clearflag.h"

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


EmberSource::EmberSource(GameObject* parent, bool active) : ParticleSource(),
parent_{ parent }, active_{ active } {}

EmberSource::~EmberSource() {}

bool EmberSource::update(RandDouble& rand, ParticleVector& particles) {
	if (parent_->tangible_ && active_) {
		if (rand() > 0.8) {
			particles.push_back(std::make_unique<FireParticle>(glm::vec3(parent_->real_pos()), ParticleTexture::SolidSquare, 0.8, 0.05, rand));
		}
	}
	return false;
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
	theta_ = rand() * 6.28318530718; // 2pi
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


DoorVortexSource::DoorVortexSource(GameObject* parent, bool active) : ParticleSource(),
parent_{ parent }, active_{ active } {}

DoorVortexSource::~DoorVortexSource() {}

bool DoorVortexSource::update(RandDouble& rand, ParticleVector& particles) {
	if (parent_->tangible_ && active_) {
		if (rand() > 0.80) {
			particles.push_back(std::make_unique<DoorVortexParticle>(glm::vec3(parent_->real_pos()), rand));
		}
	}
	return false;
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

FlagSparkleSource::FlagSparkleSource(ClearFlag* flag) {
	pos_ = glm::vec3(flag->parent_->real_pos());
	glm::vec4 c = COLOR_VECTORS[flag->color()];
	color_ = glm::vec3(c.x, c.y, c.z);
}

FlagSparkleSource::~FlagSparkleSource() {}

bool FlagSparkleSource::update(RandDouble& rand, ParticleVector& particles) {
	if (rand() > 0.95) {
		glm::vec3 d(rand() - 0.5f, rand() - 0.5f, rand() - 0.5f);
		particles.push_back(std::make_unique<FlagSparkle>(
			pos_ + glm::vec3(0.6f) * d + glm::vec3(0,0,1.2f),
			glm::vec3(0.01f) * d,
			glm::vec3(0.8f) * color_));
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


RandDouble::RandDouble() {
	std::random_device rd;
	engine_ = std::mt19937_64(rd());
	dist_ = std::uniform_real_distribution<double>(0.0, 1.0);
}

RandDouble::~RandDouble() {}

double RandDouble::operator()() {
	return dist_(engine_);
}

AnimationManager::AnimationManager(Shader* shader) :
	particle_shader_{ shader }, sounds_{ std::make_unique<SoundManager>() } {
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

	glGenTextures(1, &atlas_);
	glBindTexture(GL_TEXTURE_2D, atlas_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int width, height, channels;
	unsigned char *texture_data = stbi_load("resources/particles.png", &width, &height, &channels, STBI_rgb_alpha);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	stbi_image_free(texture_data);
}

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
	// Update particles and remove dead ones
	sources_.erase(std::remove_if(sources_.begin(), sources_.end(),
		[this](auto& p) { return p->update(rand_, particles_); }), sources_.end());
	particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
		[](auto& p) { return p->update(); }), particles_.end());
	// Run sound engine
	sounds_->flush_sounds();
}

void AnimationManager::abort_move() {
	// Cancel linear animation
	for (int i = 0; i < 6; ++i) {
		for (auto* obj : linear_animations_[i]) {
			obj->dpos_ = {};
		}
	}
	// Cancel any particles/text that shouldn't exist...?
}

void AnimationManager::reset_particles() {
	sources_.clear();
	particles_.clear();
	source_map_.clear();
}

void AnimationManager::create_bound_source(GameObject* obj, std::unique_ptr<ParticleSource> source) {
	source_map_[obj] = source.get();
	sources_.push_back(std::move(source));
}

void AnimationManager::render_particles(glm::vec3 view_dir) {
	std::vector<ParticleVertex> vertices{};
	for (auto& particle : particles_) {
		particle->get_vertex(vertices);
	}
	std::sort(vertices.begin(), vertices.end(), [view_dir](ParticleVertex a, ParticleVertex b) {
		return glm::dot((a.Position - b.Position), view_dir) < 0; });
	glBindTexture(GL_TEXTURE_2D, atlas_);
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
		if (auto* ember_source = dynamic_cast<EmberSource*>(source_map_[obj])) {
			ember_source->active_ = true;
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::IncineratorOff, obj));
			}
		}
		break;
	}
	case AnimationSignal::IncineratorOff:
	{
		if (auto* ember_source = dynamic_cast<EmberSource*>(source_map_[obj])) {
			ember_source->active_ = false;
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
		if (auto* door_source = dynamic_cast<DoorVortexSource*>(source_map_[obj])) {
			door_source->active_ = true;
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::IncineratorOff, obj));
			}
		}
		break;
	}
	case AnimationSignal::DoorOff:
	{
		if (auto* door_source = dynamic_cast<DoorVortexSource*>(source_map_[obj])) {
			door_source->active_ = false;
			if (delta_frame) {
				delta_frame->push(std::make_unique<AnimationSignalDelta>(this, AnimationSignal::IncineratorOn, obj));
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
	case AnimationSignal::FlagExists:
	{
		auto* flag = dynamic_cast<ClearFlag*>(obj->modifier());
		sources_.push_back(std::make_unique<FlagSparkleSource>(flag));
	}
	}
}


AnimationSignalDelta::AnimationSignalDelta(AnimationManager* anims, AnimationSignal signal, GameObject* obj) :
	Delta(), anims_{ anims }, signal_{ signal }, obj_{ obj } {}

AnimationSignalDelta::~AnimationSignalDelta() {}

void AnimationSignalDelta::revert() {
	anims_->receive_signal(signal_, obj_, nullptr);
}