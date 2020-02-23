#include "stdafx.h"
#include "animationmanager.h"

#include <stb_image.h>

#include "gameobject.h"
#include "texture_constants.h"

Particle::Particle() {}

Particle::~Particle() {}


const int FIRE_PART_MAX_LIFE = 30;

FireParticle::FireParticle(glm::vec3 c, RandDouble& rand): Particle() {
	pos_ = glm::vec3(c.x + (rand() - 0.5), c.y + (rand() - 0.5), c.z + 0.5);
	vel_ = glm::vec3(0.03 * (rand() - 0.5), 0.03 * (rand() - 0.5), 0.05);
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
		tex_to_vec(ParticleTexture::BlurDisk),
		glm::vec2(0.05, 0.05),
		color });
}

bool FireParticle::update() {
	pos_ += vel_;
	return --life_ == 0;
}


ParticleSource::ParticleSource() {}

ParticleSource::~ParticleSource() {}


FireParticleSource::FireParticleSource(Point3 c): ParticleSource() {
	center_ = glm::vec3(c.x, c.y, c.z);
}

FireParticleSource::~FireParticleSource() {}

void FireParticleSource::get_vertices(std::vector<ParticleVertex>& vertices) {
	for (auto& particle : particles_) {
		particle->get_vertex(vertices);
	}
}

bool FireParticleSource::update(RandDouble& rand) {
	if (rand() > 0.50) {
		particles_.push_back(std::make_unique<FireParticle>(center_, rand));
	}
	particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
		[](auto& p) { return p->update(); }), particles_.end());
	return particles_.size() == 0;
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

AnimationManager::AnimationManager(Shader* shader) : particle_shader_{ shader } {
	particle_shader_->use();
	particle_shader_->setFloat("texScale", 1.0f / PARTICLE_TEXTURE_ATLAS_SIZE);
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

	particle_sources_.push_back(std::make_unique<FireParticleSource>(Point3{ 5, 7, 5 }));
	particle_sources_.push_back(std::make_unique<FireParticleSource>(Point3{ 11, 7, 3 }));
}

AnimationManager::~AnimationManager() {
	glDeleteVertexArrays(1, &particle_VAO_);
	glDeleteBuffers(1, &particle_VBO_);
}

void AnimationManager::update() {
	for (auto* obj : moving_objects_) {
		obj->update_animation();
	}
	for (auto& source : particle_sources_) {
		source->update(rand_);
	}
}

void AnimationManager::render_particles(glm::vec3 view_dir) {
	particles_.clear();
	for (auto& source : particle_sources_) {
		source->get_vertices(particles_);
	}
	std::sort(particles_.begin(), particles_.end(), [view_dir](ParticleVertex a, ParticleVertex b) {
		return glm::dot((a.Position - b.Position), view_dir) < 0; });
	glBindTexture(GL_TEXTURE_2D, atlas_);
	glBindVertexArray(particle_VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, particle_VBO_);
	glBufferData(GL_ARRAY_BUFFER, particles_.size() * sizeof(ParticleVertex), particles_.data(), GL_STATIC_DRAW);
	glDrawArrays(GL_POINTS, 0, (GLsizei)particles_.size());
}