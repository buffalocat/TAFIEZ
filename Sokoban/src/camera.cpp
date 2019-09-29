#include "stdafx.h"

#include "camera.h"

#include "delta.h"
#include "common_constants.h"
#include "common_enums.h"
#include "roommap.h"
#include "mapfile.h"
#include "fontmanager.h"


CameraContext::CameraContext(IntRect rect, int priority) :
	rect_{ rect }, priority_{ priority } {}

CameraContext::~CameraContext() {}

void CameraContext::serialize(MapFileO& file) {}

bool CameraContext::is_null() {
	return false;
}

bool CameraContext::has_null_child() {
	return false;
}

bool CameraContext::is_free() {
	return false;
}

double* CameraContext::get_tilt_ptr() {
	return nullptr;
}

double* CameraContext::get_rot_ptr() {
	return nullptr;
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

std::string CameraContext::label() {
	return "";
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


const double VERT_ANGLE = 0.52359877559; // FOV_VERTICAL / 2.0
const double WIDTH_FACTOR = 0.76980035892; // tan(VERT_ANGLE) * ASPECT_RATIO

CamRectCalculator::CamRectCalculator(double tilt) {
	set_tilt(tilt);
}

CamRectCalculator::~CamRectCalculator() {}

void CamRectCalculator::set_tilt(double tilt) {
	tilt_ = tilt;
	hor_excess_factor_ = (WIDTH_FACTOR * cos(VERT_ANGLE) * cos(tilt)) / cos(VERT_ANGLE - tilt);
	upper_excess_factor_ = tan(VERT_ANGLE + tilt) * cos(tilt) - sin(tilt);
	lower_excess_factor_ = tan(VERT_ANGLE - tilt) * cos(tilt) + sin(tilt);
}

// TODO: account for z here?

double CamRectCalculator::compute_radius(FloatRect vis, unsigned int flags) {
	double xrad = (vis.xb + 1 - vis.xa) / (2 * hor_excess_factor_);
	double yrad = (vis.yb + 1 - vis.ya) / (upper_excess_factor_ + lower_excess_factor_);
	if (flags & CAM_FLAGS::RAD_SMALLER) {
		return std::min(xrad, yrad);
	} else if (flags & CAM_FLAGS::RAD_LARGER) {
		return std::max(xrad, yrad);
	} else if (flags & CAM_FLAGS::RAD_CLAMP_X) {
		return xrad;
	} else if (flags & CAM_FLAGS::RAD_CLAMP_Y) {
		return yrad;
	} else {
		return DEFAULT_CAM_RADIUS;
	}
}

FloatRect CamRectCalculator::compute_center_from_vis_rad(FloatRect vis, double rad) {
	return FloatRect{ vis.xa + rad * hor_excess_factor_ - 0.5, vis.ya + rad * upper_excess_factor_ - 0.5,
		vis.xb - rad * hor_excess_factor_ + 0.5, vis.yb - rad * lower_excess_factor_ + 0.5 };
}

GeneralCameraContext::GeneralCameraContext(IntRect rect, int priority, unsigned int flags) : CameraContext(rect, priority),
	flags_{ flags }, label_{},
	rad_{ DEFAULT_CAM_RADIUS }, tilt_{ DEFAULT_CAM_TILT }, rot_{ DEFAULT_CAM_ROTATION } {}

GeneralCameraContext::~GeneralCameraContext() {}

void GeneralCameraContext::serialize(MapFileO& file) {
	file << CameraCode::General;
	file << rect_ << priority_;
	file.write_uint32(flags_);
	// Choose serialization based on flags
	if (flags_ & CAM_FLAGS::NAMED_AREA) {
		file << label_;
	}
	if (flags_ & CAM_FLAGS::VIS_AUTO) {
		file << pad_.left;
		file << pad_.right;
		file << pad_.top;
		file << pad_.bottom;
	} else {
		file << visible_;
	}
	if (flags_ & CAM_FLAGS::TILT_CUSTOM) {
		file << tilt_;
	}
	if (!(flags_ & CAM_FLAGS::RAD_AUTO)) {
		file << rad_;
	}
	if (flags_ & CAM_FLAGS::ROT_CUSTOM) {
		file << rot_;
	}
}

std::unique_ptr<CameraContext> GeneralCameraContext::deserialize(MapFileI& file) {
	IntRect rect;
	int priority;
	file >> rect >> priority;
	unsigned int flags = file.read_uint32();
	auto context = std::make_unique<GeneralCameraContext>(rect, priority, flags);
	// Pull other data from file
	if (flags & CAM_FLAGS::NAMED_AREA) {
		context->label_ = file.read_str();
	}
	if (flags & CAM_FLAGS::VIS_AUTO) {
		Padding pad;
		pad.left = file.read_byte();
		pad.right = file.read_byte();
		pad.top = file.read_byte();
		pad.bottom = file.read_byte();
		context->pad_ = pad;
		context->visible_ = FloatRect{
			(float)rect.xa - pad.left,
			(float)rect.ya - pad.top,
			(float)rect.xb + pad.right,
			(float)rect.yb + pad.bottom,
		};
	} else {
		file >> context->visible_;
	}
	if (flags & CAM_FLAGS::TILT_CUSTOM) {
		file >> context->tilt_;
	}
	CamRectCalculator calc{ context->tilt_ };
	if (flags & CAM_FLAGS::RAD_AUTO) {
		if (flags & CAM_FLAGS::RAD_DEFAULT) {
			context->rad_ = DEFAULT_CAM_RADIUS;
		} else {
			context->rad_ = calc.compute_radius(context->visible_, flags);
		}
	} else {
		file >> context->rad_;
	}
	if (flags & CAM_FLAGS::CENTER_USED) {
		context->center_ = calc.compute_center_from_vis_rad(context->visible_, context->rad_);
	}
	if (flags & CAM_FLAGS::ROT_CUSTOM) {
		file >> context->rot_;
	}
	return std::move(context);
}

bool GeneralCameraContext::is_null() {
	return flags_ & CAM_FLAGS::NULL_AREA;
}

bool GeneralCameraContext::has_null_child() {
	return flags_ & CAM_FLAGS::HAS_NULL_CHILD;
}

bool GeneralCameraContext::is_free() {
	return flags_ & CAM_FLAGS::FREE_CAM;
}

bool GeneralCameraContext::can_override_free() {
	return true;
}

std::string GeneralCameraContext::label() {
	if (flags_ & CAM_FLAGS::NAMED_AREA) {
		return label_;
	}
	return "";
}

FPoint3 GeneralCameraContext::center(FPoint3 pos) {
	FPoint3 c = pos;
	if (flags_ & CAM_FLAGS::POS_CLAMP_X) {
		c.x = std::min(std::max(c.x, center_.xa), center_.xb);
	} else if (flags_ & CAM_FLAGS::POS_FIX_X) {
		c.x = (center_.xa + center_.xb) / 2.0;
	}
	if (flags_ & CAM_FLAGS::POS_CLAMP_Y) {
		c.y = std::min(std::max(c.y, center_.ya), center_.yb);
	} else if (flags_ & CAM_FLAGS::POS_FIX_Y) {
		c.y = (center_.ya + center_.yb) / 2.0;
	}
	return c;
}

double GeneralCameraContext::radius(FPoint3 pos) {
	return rad_;
}

double GeneralCameraContext::tilt(FPoint3 pos) {
	return tilt_;
}

double GeneralCameraContext::rotation(FPoint3 pos) {
	return rot_;
}

void GeneralCameraContext::shift_by(Point3 d, int width, int height) {
	CameraContext::shift_by(d, width, height);
	visible_.xa += d.x;
	visible_.xb += d.x;
	visible_.ya += d.y;
	visible_.yb += d.y;
	center_.xa += d.x;
	center_.xb += d.x;
	center_.ya += d.y;
	center_.yb += d.y;
}

double* GeneralCameraContext::get_tilt_ptr() {
	return &tilt_;
}

double* GeneralCameraContext::get_rot_ptr() {
	return &rot_;
}

DependentNullCameraContext::DependentNullCameraContext(IntRect rect, int priority) :
	CameraContext(rect, priority) {}

DependentNullCameraContext::~DependentNullCameraContext() {}

bool DependentNullCameraContext::is_null() {
	return true;
}

Camera::Camera(int w, int h) :
	active_label_{}, label_display_cooldown_{},
	width_{ w }, height_{ h },
	default_context_{ GeneralCameraContext(IntRect{0,0,w - 1,h - 1}, 0, CAM_FLAGS::DEFAULT) },
	free_context_{ GeneralCameraContext(IntRect{0,0,w - 1,h - 1}, 0, CAM_FLAGS::FREE_CAM) },
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
	if (context->has_null_child()) {
		IntRect border_rect{ std::max(0, rect.xa - 1), std::max(0, rect.ya - 1),
				std::min(width_ - 1, rect.xb + 1), std::min(height_ - 1, rect.yb + 1) };
		push_context(std::make_unique<DependentNullCameraContext>(border_rect, priority - 1));
	}
	loaded_contexts_.push_back(std::move(context));
}

// Note: it just gets replaced with the default context (until the map is reloaded)
void Camera::remove_context(CameraContext* context) {
	IntRect rect = context->rect_;
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

bool Camera::update_context(Point3 vpos) {
	auto* new_context = context_map_[vpos.x][vpos.y];
	if (!new_context->is_null()) {
		auto* old_context = context_;
		context_ = new_context;
		return old_context != new_context;
	} else {
		return false;
	}
}

void Camera::set_target(FPoint3 rpos) {
	CameraContext* context = free_override_ ? &free_context_ : context_;
	target_pos_ = context->center(rpos);
	target_rad_ = context->radius(rpos);
	target_tilt_ = context->tilt(rpos);
	target_rot_ = context->rotation(rpos);
}

bool Camera::update_label() {
	std::string label = context_->label();
	if (active_label_ != label) {
		active_label_ = label;
		return !label.empty();
	}
	return false;
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
	double drot = cur_rot_ - target_rot_;
	if (drot > TWO_PI / 2.0) {
		cur_rot_ -= TWO_PI;
	} else if (drot < -TWO_PI / 2.0) {
		cur_rot_ += TWO_PI;
	}
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

void Camera::handle_free_cam_input(GLFWwindow* window) {
	double* tilt_ptr = nullptr;
	double* rot_ptr = nullptr;
	if (context_->is_free()) {
		tilt_ptr = context_->get_tilt_ptr();
		rot_ptr = context_->get_rot_ptr();
	}
	if (free_override_) {
		tilt_ptr = free_context_.get_tilt_ptr();
		rot_ptr = free_context_.get_rot_ptr();
	}
	if (tilt_ptr) {
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			*tilt_ptr = std::min(*tilt_ptr + FREE_CAM_TILT_SPEED, MAX_CAM_TILT);
		} else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			*tilt_ptr = std::max(*tilt_ptr - FREE_CAM_TILT_SPEED, MIN_CAM_TILT);
		}
	}
	if (rot_ptr) {
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			*rot_ptr += FREE_CAM_ROT_SPEED;
			if (*rot_ptr >= TWO_PI) {
				*rot_ptr -= TWO_PI;
			}
		} else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			*rot_ptr -= FREE_CAM_ROT_SPEED;
			if (*rot_ptr <= -TWO_PI) {
				*rot_ptr += TWO_PI;
			}
		}
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
