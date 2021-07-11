#include "stdafx.h"
#include "background.h"

#include "common_constants.h"
#include "point.h"


BackgroundAnimation::BackgroundAnimation() {
	shader_.use();
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	vert_data_ = generate_sphere_verts(20, 20);
	/*vert_data_ = {
	glm::vec3(-1.0f, -1.0f, 0.0f),
	glm::vec3(-1.0f,  1.0f, 0.0f),
	glm::vec3(1.0f,  1.0f, 0.0f),
	glm::vec3(1.0f, -1.0f, 0.0f),
	};
	std::cout << vert_data_.size() * sizeof(glm::vec3) << std::endl;*/
	glBufferData(GL_ARRAY_BUFFER, vert_data_.size() * sizeof(glm::vec3), vert_data_.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}

BackgroundAnimation::~BackgroundAnimation() {
	glDeleteVertexArrays(1, &vao_);
	glDeleteBuffers(1, &vbo_);
}

void BackgroundAnimation::snap_colors() {
	compute_cur_color();
	cur_color_up_ = target_color_up_;
	cur_color_down_ = target_color_down_;
}


const glm::vec4 HUB_ALPHA_UP(1.0, 0.3, 0.3, 1.0);
const glm::vec4 HUB_ALPHA_DOWN(1.0, 0.55, 0.55, 1.0);
const glm::vec4 HUB_BETA_UP(0.5, 0.85, 0.55, 1.0);
const glm::vec4 HUB_BETA_DOWN(0.6, 0.95, 0.65, 1.0);
const glm::vec4 HUB_GAMMA_UP(0.3, 0.4, 1.0, 1.0);
const glm::vec4 HUB_GAMMA_DOWN(0.5, 0.7, 1.0, 1.0);
const glm::vec4 HUB_DELTA_UP(0.7, 0.55, 0.9, 1.0);
const glm::vec4 HUB_DELTA_DOWN(0.85, 0.7, 1.0, 1.0);
const glm::vec4 POST_GATE_UP(0.8, 0.0, -1.5, 1.0);
const glm::vec4 POST_GATE_DOWN(1.0, 0.9, 0.8, 1.0);
const glm::vec4 NEUTRAL_UP(1.0, 1.0, 1.0, 1.0);
const glm::vec4 NEUTRAL_DOWN(0.9, 0.9, 0.9, 1.0);


void BackgroundAnimation::compute_cur_color() {
	switch (type_) {
	case BackgroundSpecType::Default:
	default:
	{
		target_color_up_ = color_up_;
		target_color_down_ = color_down_;
		break;
	}
	case BackgroundSpecType::HubAlpha:
	{
		if (cur_pos_.y > 43) {
			target_color_up_ = POST_GATE_UP;
			target_color_down_ = POST_GATE_DOWN;
		} else {
			target_color_up_ = HUB_ALPHA_UP;
			target_color_down_ = HUB_ALPHA_DOWN;
		}
		break;
	}
	case BackgroundSpecType::HubBeta:
	{
		target_color_up_ = HUB_BETA_UP;
		target_color_down_ = HUB_BETA_DOWN;
		break;
	}
	case BackgroundSpecType::HubGamma:
	{
		target_color_up_ = HUB_GAMMA_UP;
		target_color_down_ = HUB_GAMMA_DOWN;
		break;
	}
	case BackgroundSpecType::HubDelta:
	{
		if (cur_pos_.y > 48 && cur_pos_.x > 28 && cur_pos_.x < 36) {
			target_color_up_ = NEUTRAL_UP;
			target_color_down_ = NEUTRAL_DOWN;
		} else {
			target_color_up_ = HUB_DELTA_UP;
			target_color_down_ = HUB_DELTA_DOWN;
		}
		break;
	}
	case BackgroundSpecType::Nexus:
	{
		float dx = cur_pos_.x - 20;
		float dy = cur_pos_.y - 20;
		float weight = 3;
		glm::vec4 up = weight * NEUTRAL_UP;
		glm::vec4 down = weight * NEUTRAL_DOWN;
		float dw;
		const float null_range = 1;
		if (dy < -null_range) {
			dw = -null_range - dy;
			weight += dw;
			up += dw * HUB_ALPHA_UP;
			down += dw * HUB_ALPHA_DOWN;
		} else if (dy > null_range) {
			dw = dy - null_range;
			weight += dw;
			up += dw * HUB_GAMMA_UP;
			down += dw * HUB_GAMMA_DOWN;
		}
		if (dx < -null_range) {
			dw = -null_range - dx;
			weight += dw;
			up += dw * HUB_DELTA_UP;
			down += dw * HUB_DELTA_DOWN;
		} else if (dx > null_range) {
			dw = dx - null_range;
			weight += dw;
			up += dw * HUB_BETA_UP;
			down += dw * HUB_BETA_DOWN; 
		}
		target_color_up_ = up / weight;
		target_color_down_ = down / weight;
		break;
	}
	case BackgroundSpecType::HFlag:
	{
		target_color_up_ = NEUTRAL_UP;
		target_color_down_ = NEUTRAL_DOWN;
		break;
	}
	case BackgroundSpecType::XNexus:
	{
		float dx = cur_pos_.x - 28;
		float dy = cur_pos_.y - 22;
		float weight = 3;
		glm::vec4 up = weight * NEUTRAL_UP;
		glm::vec4 down = weight * NEUTRAL_DOWN;
		float dw;
		const float null_range = 1;
		if (dy < -null_range) {
			dw = -null_range - dy;
			weight += dw;
			up += dw * HUB_BETA_UP;
			down += dw * HUB_BETA_DOWN;
		} else if (dy > null_range) {
			dw = dy - null_range;
			weight += dw;
			up += dw * HUB_DELTA_UP;
			down += dw * HUB_DELTA_DOWN;
		}
		if (dx < -null_range) {
			dw = -null_range - dx;
			weight += dw;
			up += dw * HUB_ALPHA_UP;
			down += dw * HUB_ALPHA_DOWN;
		} else if (dx > null_range) {
			dw = dx - null_range;
			weight += dw;
			up += dw * HUB_GAMMA_UP;
			down += dw * HUB_GAMMA_DOWN;
		}
		target_color_up_ = up / weight;
		target_color_down_ = down / weight;
		break;
	}
	case BackgroundSpecType::AlphaSimple:
	{
		target_color_up_ = HUB_ALPHA_UP;
		target_color_down_ = HUB_ALPHA_DOWN;
		break;
	}
	}
}


void BackgroundAnimation::set_positions(glm::vec3 camera_sight, glm::vec3 camera_up, FPoint3 player_pos) {
	auto view = glm::lookAt(glm::vec3(0.0f), -camera_sight, camera_up);
	auto projection = glm::perspective((float)FOV_VERTICAL, (float)ASPECT_RATIO, 0.1f, 10.0f);
	shader_.use();
	auto PV = projection * view;
	shader_.setMat4("PV", PV);
	cur_pos_ = player_pos;
	compute_cur_color();
}

const float COLOR_DAMP = 0.1;

void BackgroundAnimation::update() {
	cur_color_down_ += COLOR_DAMP * (target_color_down_ - cur_color_down_ );
	cur_color_up_ += COLOR_DAMP * (target_color_up_ - cur_color_up_);
}

void BackgroundAnimation::draw() {
	shader_.use();
	shader_.setVec4("color_up", cur_color_up_);
	shader_.setVec4("color_down", cur_color_down_);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vert_data_.size());
}


std::vector<glm::vec3> generate_sphere_verts(int n_theta, int n_phi) {
	std::vector<glm::vec3> verts{};
	verts.reserve(2 * (n_theta + 1) * n_phi);
	float d_theta = TWO_PI / n_theta;
	float d_phi = TWO_PI / (2 * n_phi);
	float x, y, low_z, low_r, phi;
	float high_z = -1;
	float high_r = 0;
	float R = 1.0;
	for (int i_phi = 0; i_phi < n_phi; ++i_phi) {
		low_z = high_z;
		low_r = high_r;
		phi = (i_phi + 1) * d_phi;
		high_z = -cos(phi);
		high_r = sin(phi);
		for (int i_theta = 0; i_theta <= n_theta; ++i_theta) {
			x = cos(i_theta * d_theta);
			y = sin(i_theta * d_theta);
			verts.push_back(R * glm::vec3(high_r * x, high_r * y, high_z));
			verts.push_back(R * glm::vec3(low_r * x, low_r * y, low_z));
		}
	}
	return verts;
}
