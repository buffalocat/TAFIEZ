#include "stdafx.h"

#include "camera.h"

#include "delta.h"
#include "graphicsmanager.h"
#include "common_constants.h"
#include "common_enums.h"
#include "roommap.h"
#include "mapfile.h"

CameraContext::CameraContext(std::string label, IntRect rect, int priority, bool named_area, bool null_child) :
	label_{ label }, rect_{ rect }, priority_{ priority }, named_area_{ named_area }, null_child_{ null_child } {}

CameraContext::~CameraContext() {}

bool CameraContext::is_null() {
	return false;
}

FPoint3 CameraContext::center(FPoint3 pos) {
	return pos;
}

double CameraContext::radius(FPoint3 pos) {
	return DEFAULT_CAM_RADIUS;
}

double CameraContext::tilt(FPoint3 pos) {
	return DEFAULT_CAM_TILT;
}

double CameraContext::rotation(FPoint3 pos) {
	return DEFAULT_CAM_ROTATION;
}

void CameraContext::shift_by(Point3 d, int width, int height) {
	rect_.xa += d.x;
	clamp(&rect_.xa, 0, width - 1);
	rect_.xb += d.x;
	clamp(&rect_.xb, 0, width - 1);
	rect_.ya += d.y;
	clamp(&rect_.ya, 0, height - 1);
	rect_.yb += d.y;
	clamp(&rect_.yb, 0, height - 1);
}

void CameraContext::extend_by(Point3 d, int width, int height) {
	clamp(&rect_.xa, 0, width - 1);
	clamp(&rect_.xb, 0, width - 1);
	clamp(&rect_.ya, 0, height - 1);
	clamp(&rect_.yb, 0, height - 1);
}

ClampedCameraContext::ClampedCameraContext(std::string label, IntRect rect, int priority, bool named_area, bool null_child,
	double radius, double tilt, FloatRect center) :
	CameraContext(label, rect, priority, named_area, null_child),
	rad_{ radius }, tilt_{ tilt }, center_{ center } {}

ClampedCameraContext::~ClampedCameraContext() {}

FPoint3 ClampedCameraContext::center(FPoint3 pos) {
	return {
		std::min(std::max(pos.x, center_.xa), center_.xb),
		std::min(std::max(pos.y, center_.ya), center_.yb),
		pos.z,
	};
}

double ClampedCameraContext::radius(FPoint3 pos) {
	return rad_;
}

double ClampedCameraContext::tilt(FPoint3 pos) {
	return tilt_;
}

void ClampedCameraContext::serialize(MapFileO& file) {
	file << CameraCode::Clamped;
	file << label_ << rect_ << priority_ << named_area_ << null_child_;
	file << rad_ << tilt_ << center_;
}

CameraContext* ClampedCameraContext::deserialize(MapFileI& file) {
	std::string label = file.read_str();
	IntRect rect;
	int priority;
	bool named_area, null_child;
	double rad, tilt;
	FloatRect vis;
	file >> rect >> priority >> named_area >> null_child >> rad >> tilt >> vis;
	return new ClampedCameraContext(label, rect, priority, named_area, null_child, rad, tilt, vis);
}

void ClampedCameraContext::shift_by(Point3 d, int width, int height) {
	CameraContext::shift_by(d, width, height);
	center_.xa += d.x;
	center_.xb += d.x;
	center_.ya += d.y;
	center_.yb += d.y;
}


NullCameraContext::NullCameraContext(std::string label, IntRect rect, int priority, bool named_area, bool independent) :
	CameraContext(label, rect, priority, named_area, false), independent_{ independent } {}

NullCameraContext::~NullCameraContext() {}

bool NullCameraContext::is_null() {
	return true;
}

void NullCameraContext::serialize(MapFileO& file) {
	if (independent_) {
		file << (unsigned char)CameraCode::Null;
		file << label_ << rect_ << named_area_ << priority_;
	}
}

CameraContext* NullCameraContext::deserialize(MapFileI& file) {
	std::string label = file.read_str();
	IntRect rect;
	int priority;
	bool named_area;
	file >> rect >> priority >> named_area;
	return new NullCameraContext(label, rect, priority, named_area, true);
}


Camera::Camera(int w, int h) :
	active_label_{}, label_display_cooldown_{},
	width_{ w }, height_{ h },
	default_context_{ ClampedCameraContext("DEFAULT", IntRect{0,0,w - 1,h - 1}, 0, false, false,
		DEFAULT_CAM_RADIUS, DEFAULT_CAM_TILT, FloatRect{0,0,w - 1,h - 1}) },
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
	IntRect rect = context->rect_;
	int priority = context->priority_;
	for (int i = rect.xa; i <= rect.xb; ++i) {
		for (int j = rect.ya; j <= rect.yb; ++j) {
			if (priority > context_map_[i][j]->priority_) {
				context_map_[i][j] = context.get();
			}
		}
	}
	if (context->null_child_) {
		IntRect border_rect{ std::max(0, rect.xa - 1), std::max(0, rect.ya - 1),
			std::min(width_ - 1, rect.xb + 1), std::min(height_ - 1, rect.yb + 1) };
		push_context(std::make_unique<NullCameraContext>(context->label_ + "(NULL)", border_rect, priority - 1, false, false));
	}
	loaded_contexts_.push_back(std::move(context));
}

// Note: it just gets replaced with the default context (until the map is reloaded)
void Camera::remove_context(CameraContext* context) {
	IntRect rect = context->rect_;
	int priority = context->priority_;
	for (int i = rect.xa; i <= rect.xb; ++i) {
		for (int j = rect.ya; j <= rect.yb; ++j) {
			if (context_map_[i][j] == context) {
				context_map_[i][j] = &default_context_;
			}
		}
	}
	loaded_contexts_.erase(std::remove_if(loaded_contexts_.begin(), loaded_contexts_.end(),
		[context](std::unique_ptr<CameraContext>& unique) {return unique.get() == context; }), loaded_contexts_.end());
}

double Camera::get_radius() {
	return cur_rad_;
}

FPoint3 Camera::get_pos() {
	return cur_pos_;
}

double Camera::get_tilt() {
	return cur_tilt_;
}

double Camera::get_rotation() {
	return cur_rot_;
}

void Camera::set_target(Point3 vpos, FPoint3 rpos) {
	CameraContext* new_context = context_map_[vpos.x][vpos.y];
	if (new_context->named_area_) {
		update_label(new_context->label_);
	}
	if (!new_context->is_null()) {
		context_ = new_context;
	}
	target_pos_ = context_->center(rpos);
	target_rad_ = context_->radius(rpos);
	target_tilt_ = context_->tilt(rpos);
	target_rot_ = context_->rotation(rpos);
}

void Camera::update_label(std::string label) {
	if (!label.empty() && active_label_ != label) {
		active_label_ = label;
		label_display_cooldown_ = AREA_NAME_DISPLAY_FRAMES;
	}
}

void Camera::draw_label(GraphicsManager* gfx) {
	if (label_display_cooldown_ == 0) {
		return;
	}
	--label_display_cooldown_;
	float opacity = 0;
	if (label_display_cooldown_ > AREA_NAME_DISPLAY_FRAMES - AREA_NAME_FADE_FRAMES) {
		opacity = (float)(AREA_NAME_DISPLAY_FRAMES - label_display_cooldown_) / (float)AREA_NAME_FADE_FRAMES;
	} else if (label_display_cooldown_ < AREA_NAME_FADE_FRAMES) {
		opacity = (float)label_display_cooldown_ / (float)AREA_NAME_FADE_FRAMES;
	} else {
		opacity = 1;
	}
	gfx->render_text(active_label_, opacity);
}

void Camera::set_current_to_target() {
	cur_pos_ = target_pos_;
	cur_rad_ = target_rad_;
	cur_tilt_ = target_tilt_;
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

void Camera::shift_by(Point3 d) {
	width_ += d.x;
	height_ += d.y;
	for (auto& context : loaded_contexts_) {
		context->shift_by(d, width_, height_);
	}
}

void Camera::extend_by(Point3 d) {
	width_ += d.x;
	height_ += d.y;
	for (auto& context : loaded_contexts_) {
		context->extend_by(d, width_, height_);
	}
}

// We have a few magic numbers for tweaking camera smoothness
// This function may be something more interesting than exponential damping later
double damp_avg(double target, double cur) {
	if (fabs(target - cur) <= 0.0001f) {
		return target;
	} else {
		return (target + 5 * cur) / 6.0f;
	}
}
