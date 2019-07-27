#include "stdafx.h"
#include <cmath>

#include "camera.h"

#include "common_constants.h"
#include "common_enums.h"
#include "roommap.h"
#include "mapfile.h"

CameraContext::CameraContext(std::string label, int x, int y, int w, int h, int priority) :
	label_{ label }, x_{ x }, y_{ y }, w_{ w }, h_{ h }, priority_{ priority } {}

CameraContext::~CameraContext() {}

bool CameraContext::is_null() {
	return false;
}

FPoint3 CameraContext::center(FPoint3 pos) {
	return pos;
}

float CameraContext::radius(FPoint3 pos) {
	return DEFAULT_CAM_RADIUS;
}

float CameraContext::tilt(FPoint3 pos) {
	return DEFAULT_CAM_TILT;
}

float CameraContext::rotation(FPoint3 pos) {
	return DEFAULT_CAM_ROTATION;
}

void CameraContext::serialize(MapFileO& file) {
	file << label_;
	file << x_;
	file << y_;
	file << w_;
	file << h_;
	file << priority_;
}

FreeCameraContext::FreeCameraContext(std::string label, int x, int y, int w, int h, int priority, float radius, float tilt, float rotation) :
	CameraContext(label, x, y, w, h, priority), rad_{ radius }, tilt_{ tilt }, rot_{ rotation } {}

FreeCameraContext::~FreeCameraContext() {}

FPoint3 FreeCameraContext::center(FPoint3 pos) {
	return pos;
}

float FreeCameraContext::radius(FPoint3 pos) {
	return rad_;
}

float FreeCameraContext::tilt(FPoint3 pos) {
	return tilt_;
}

float FreeCameraContext::rotation(FPoint3 pos) {
	return rot_;
}

void FreeCameraContext::serialize(MapFileO& file) {
	file << CameraCode::Free;
	CameraContext::serialize(file);
	file << rad_;
	file << tilt_;
	file << rot_;
}

CameraContext* FreeCameraContext::deserialize(MapFileI& file) {
	std::string label = file.read_str();
	int x, y, w, h, p;
	float rad, tilt, rot;
	file >> x >> y >> w >> h >> p >> rad >> tilt >> rot;
	return new FreeCameraContext(label, x, y, w, h, p, rad, tilt, rot);
}

void FreeCameraContext::change_rotation(float dr) {
	rot_ += dr;
}


FixedCameraContext::FixedCameraContext(std::string label, int x, int y, int w, int h, int priority, float radius, float tilt, float rotation, FPoint3 center) :
	CameraContext(label, x, y, w, h, priority), rad_{ radius }, tilt_{ tilt }, rot_{ rotation }, center_{ center } {}

FixedCameraContext::~FixedCameraContext() {}

FPoint3 FixedCameraContext::center(FPoint3 pos) {
	return center_ + FPoint3{ -0.5, -0.5, -0.5 };
}

float FixedCameraContext::radius(FPoint3 pos) {
	return rad_;
}

float FixedCameraContext::tilt(FPoint3 pos) {
	return tilt_;
}

float FixedCameraContext::rotation(FPoint3 pos) {
	return rot_;
}

void FixedCameraContext::serialize(MapFileO& file) {
	file << CameraCode::Fixed;
	CameraContext::serialize(file);
	file << rad_;
	file << tilt_;
	file << rot_;
	file << center_;
}

CameraContext* FixedCameraContext::deserialize(MapFileI& file) {
	std::string label = file.read_str();
	int x, y, w, h, p;
	float rad, tilt, rot;
	FPoint3 center{};
	file >> x >> y >> w >> h >> p >> rad >> tilt >> rot >> center;
	return new FixedCameraContext(label, x, y, w, h, p, rad, tilt, rot, center);
}

ClampedCameraContext::ClampedCameraContext(std::string label, int x, int y, int w, int h, int priority, float radius, float tilt, int xpad, int ypad) :
	CameraContext(label, x, y, w, h, priority), rad_{ radius }, xpad_{ xpad }, ypad_{ ypad } {}

ClampedCameraContext::~ClampedCameraContext() {}

FPoint3 ClampedCameraContext::center(FPoint3 pos) {
	return {
		std::min(std::max(pos.x, (float)x_ + xpad_), (float)x_ + w_ - xpad_),
		std::min(std::max(pos.y, (float)y_ + ypad_), (float)y_ + h_ - ypad_),
		pos.z
	};
}

float ClampedCameraContext::radius(FPoint3 pos) {
	return rad_;
}

float ClampedCameraContext::tilt(FPoint3 pos) {
	return tilt_;
}

void ClampedCameraContext::serialize(MapFileO& file) {
	file << CameraCode::Clamped;
	CameraContext::serialize(file);
	file << rad_;
	file << tilt_;
	file << xpad_;
	file << ypad_;
}

CameraContext* ClampedCameraContext::deserialize(MapFileI& file) {
	std::string label = file.read_str();
	int x, y, w, h, p, xpad, ypad;
	float rad, tilt;
	file >> x >> y >> w >> h >> p >> rad >> tilt >> xpad >> ypad;
	return new ClampedCameraContext(label, x, y, w, h, p, rad, tilt, xpad, ypad);
}

NullCameraContext::NullCameraContext(std::string label, int x, int y, int w, int h, int priority) :
	CameraContext(label, x, y, w, h, priority) {}

NullCameraContext::~NullCameraContext() {}

bool NullCameraContext::is_null() {
	return true;
}

void NullCameraContext::serialize(MapFileO& file) {
	file << (unsigned char)CameraCode::Null;
	CameraContext::serialize(file);
}

CameraContext* NullCameraContext::deserialize(MapFileI& file) {
	std::string label = file.read_str();
	int x, y, w, h, p;
	file >> x >> y >> w >> h >> p;
	return new NullCameraContext(label, x, y, w, h, p);
}


Camera::Camera(int w, int h) : width_{ w }, height_{ h },
default_context_{ FreeCameraContext("DEFAULT", 0, 0, w, h, 0, DEFAULT_CAM_RADIUS, DEFAULT_CAM_TILT, DEFAULT_CAM_ROTATION) },
context_{}, loaded_contexts_{},
context_map_{},
target_pos_{ FPoint3{0,0,0} }, cur_pos_{ FPoint3{0,0,0} },
target_rad_{ DEFAULT_CAM_RADIUS }, cur_rad_{ DEFAULT_CAM_RADIUS },
target_tilt_{ DEFAULT_CAM_TILT }, cur_tilt_{ DEFAULT_CAM_TILT },
target_rot_{ DEFAULT_CAM_ROTATION }, cur_rot_{ DEFAULT_CAM_ROTATION }
{
	context_ = &default_context_;
	context_map_ = std::vector<std::vector<CameraContext*>>(w, std::vector<CameraContext*>(h, context_));
}

void Camera::serialize(MapFileO& file) {
	file << MapCode::CameraRects;
	for (auto& context : loaded_contexts_) {
		context->serialize(file);
	}
	file << CameraCode::NONE;
}

void Camera::push_context(std::unique_ptr<CameraContext> context) {
	int left = context->x_;
	int right = left + context->w_;
	int top = context->y_;
	int bottom = top + context->h_;
	int priority = context->priority_;
	for (int i = left; i < right; ++i) {
		for (int j = top; j < bottom; ++j) {
			if (priority > context_map_[i][j]->priority_) {
				context_map_[i][j] = context.get();
			}
		}
	}
	loaded_contexts_.push_back(std::move(context));
}

// Note: it just gets replaced with the default context (until the map is reloaded)
void Camera::remove_context(CameraContext* context) {
	int left = context->x_;
	int right = left + context->w_;
	int top = context->y_;
	int bottom = top + context->h_;
	for (int i = left; i < right; ++i) {
		for (int j = top; j < bottom; ++j) {
			if (context_map_[i][j] == context) {
				context_map_[i][j] = &default_context_;
			}
		}
	}
	loaded_contexts_.erase(std::remove_if(loaded_contexts_.begin(), loaded_contexts_.end(),
		[context](std::unique_ptr<CameraContext>& unique) {return unique.get() == context; }), loaded_contexts_.end());
}

float Camera::get_radius() {
	return cur_rad_;
}

FPoint3 Camera::get_pos() {
	return cur_pos_;
}

float Camera::get_tilt() {
	return cur_tilt_;
}

float Camera::get_rotation() {
	return cur_rot_;
}

void Camera::change_rotation(float dr) {
	if (auto* free = dynamic_cast<FreeCameraContext*>(context_)) {
		free->change_rotation(dr);
	}
}

void Camera::set_target(Point3 vpos, FPoint3 rpos) {
	CameraContext* new_context = context_map_[vpos.x][vpos.y];
	if (!new_context->is_null()) {
		context_ = new_context;
	}
	target_pos_ = context_->center(rpos);
	target_rad_ = context_->radius(rpos);
	target_tilt_ = context_->tilt(rpos);
	target_rot_ = context_->rotation(rpos);
}

void Camera::set_current_pos(Point3 pos) {
	CameraContext* new_context = context_map_[pos.x][pos.y];
	if (!new_context->is_null()) {
		context_ = new_context;
	}
	target_pos_ = context_->center(pos);
	cur_pos_ = target_pos_;
	target_rad_ = context_->radius(pos);
	cur_rad_ = target_rad_;
	target_tilt_ = context_->tilt(pos);
	cur_tilt_ = target_tilt_;
	target_rot_ = context_->rotation(pos);
	cur_rot_ = target_rot_;
}

void Camera::update() {
	cur_pos_ = FPoint3{ damp_avg(target_pos_.x, cur_pos_.x), damp_avg(target_pos_.y, cur_pos_.y), damp_avg(target_pos_.z, cur_pos_.z) };
	cur_rad_ = damp_avg(target_rad_, cur_rad_);
	cur_tilt_ = damp_avg(target_tilt_, cur_tilt_);
	cur_rot_ = damp_avg(target_rot_, cur_rot_);
}

std::vector<std::unique_ptr<CameraContext>>& Camera::loaded_contexts() {
	return loaded_contexts_;
}

// We have a few magic numbers for tweaking camera smoothness
// This function may be something more interesting than exponential damping later
float damp_avg(float target, float cur) {
	if (fabs(target - cur) <= 0.0001f) {
		return target;
	} else {
		return (target + 11 * cur) / 12.0f;
	}
}
