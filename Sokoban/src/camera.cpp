#include "stdafx.h"

#include "camera.h"

#include "delta.h"
#include "common_constants.h"
#include "common_enums.h"
#include "roommap.h"
#include "mapfile.h"
#include "fontmanager.h"
#include "gamestate.h"


CameraContext::CameraContext(IntRect rect, int priority, std::string label) :
	rect_{ rect }, priority_{ priority }, label_{ label } {}

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

std::string CameraContext::area_name() {
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

const double CLAMP_EPSILON = 0.5;

FloatRect CamRectCalculator::compute_center_from_vis_rad(FloatRect vis, double rad) {
	double xa = vis.xa + rad * hor_excess_factor_ - 0.5;
	double xb = vis.xb - rad * hor_excess_factor_ + 0.5;
	double ya = vis.ya + rad * upper_excess_factor_ - 0.5;
	double yb = vis.yb - rad * lower_excess_factor_ + 0.5;
	if (xb - xa < CLAMP_EPSILON) {
		xa = xb = (xa + xb) / 2.0;
	}
	if (yb - ya < CLAMP_EPSILON) {
		ya = yb = (ya + yb) / 2.0;
	}
	return FloatRect{ xa, ya, xb, yb };
}

GeneralCameraContext::GeneralCameraContext(IntRect rect, int priority, std::string label, unsigned int flags) : CameraContext(rect, priority, label),
	flags_{ flags }, rad_{ DEFAULT_CAM_RADIUS }, tilt_{ DEFAULT_CAM_TILT }, rot_{ DEFAULT_CAM_ROTATION } {}

GeneralCameraContext::~GeneralCameraContext() {}

void GeneralCameraContext::serialize(MapFileO& file) {
	file << CameraCode::General;
	file << rect_ << priority_;
	file.write_uint32(flags_);
	file << label_;
	// Choose serialization based on flags
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
	if (flags_ & CAM_FLAGS::CENTER_CUSTOM) {
		file << center_point_;
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
	std::string label = file.read_str();
	auto context = std::make_unique<GeneralCameraContext>(rect, priority, label, flags);
	
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
	if (flags & CAM_FLAGS::CENTER_CUSTOM) {
		file >> context->center_point_;
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

std::string GeneralCameraContext::area_name() {
	if (flags_ & CAM_FLAGS::NAMED_AREA) {
		return label_;
	}
	return "";
}

FPoint3 GeneralCameraContext::center(FPoint3 pos) {
	FPoint3 c = pos;
	if (flags_ & CAM_FLAGS::CENTER_CUSTOM) {
		return pos + center_point_;
	}
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
	if (flags_ & CAM_FLAGS::CIRC_CAMERA) {
		double dx = (pos.x - (rect_.xa + rect_.xb) / 2.0);
		double dy = -(pos.y - (rect_.ya + rect_.yb) / 2.0);
		return atan2(dx*dx*dx, dy*dy*dy);
	} else {
		return rot_;
	}
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

DependentNullCameraContext::DependentNullCameraContext(IntRect rect, int priority, std::string label) :
	CameraContext(rect, priority, label) {}

DependentNullCameraContext::~DependentNullCameraContext() {}

bool DependentNullCameraContext::is_null() {
	return true;
}

Camera::Camera(int w, int h) :
	active_label_{}, label_display_cooldown_{},
	width_{ w }, height_{ h },
	default_context_{ GeneralCameraContext(IntRect{0,0,w - 1,h - 1}, 0, "DEFAULT", CAM_FLAGS::DEFAULT) },
	free_context_{ GeneralCameraContext(IntRect{0,0,w - 1,h - 1}, 0, "FREE", CAM_FLAGS::FREE_CAM) },
	context_{}, loaded_contexts_{},
	context_map_{},
	vpos_{ Point3{0,0,0} }, target_pos_{ FPoint3{0,0,0} }, cur_pos_{ FPoint3{0,0,0} },
	target_rad_{ DEFAULT_CAM_RADIUS }, cur_rad_{ DEFAULT_CAM_RADIUS },
	target_tilt_{ DEFAULT_CAM_TILT }, cur_tilt_{ DEFAULT_CAM_TILT },
	target_rot_{ DEFAULT_CAM_ROTATION }, cur_rot_{ DEFAULT_CAM_ROTATION }
{
	context_ = free_override_ ? &free_context_ : &default_context_;
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
		push_context(std::make_unique<DependentNullCameraContext>(border_rect, priority - 1, context->label_ + " (NULL)"));
	}
	loaded_contexts_.push_back(std::move(context));
}

std::pair<int, bool> Camera::compute_direction_index(int i) {
	if (auto* gcc = dynamic_cast<GeneralCameraContext*>(context_)) {
		if (gcc->flags_ & CAM_FLAGS::CIRC_CAMERA) {
			int dx = vpos_.x - 20;
			int dy = vpos_.y - 20;
			int di = 0;
			bool is_left_right = (i % 2 == 0);
			bool is_not_corner = true;
			if (dx > -dy) {
				if (dx > dy) {
					di = 1;
				} else if (dx < dy) {
					di = 2;
				} else {
					is_not_corner = false;
					if (i == 0) {
						di = 2;
					} else {
						di = 1;
					}
				}
			} else if (dx < -dy) {
				if (dx > dy) {
					di = 0;
				} else if (dx < dy) {
					di = 3;
				} else {
					is_not_corner = false;
					if (i == 0) {
						di = 0;
					} else {
						di = 3;
					}
				}
			} else {
				is_not_corner = false;
				if (dx > dy) {
					if (i == 0) {
						di = 1;
					} else {
						di = 0;
					}
				} else if (dx < dy) {
					if (i == 0) {
						di = 3;
					} else {
						di = 2;
					}
				}
			}
			return std::make_pair((i + di) % 4, is_left_right || is_not_corner);
		}
	}
	const double HALF_PI = 1.57079632679;
	double angle = get_rotation();
	return std::make_pair((i + (int)((angle + 4.5 * HALF_PI) / HALF_PI)) % 4, true);
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

bool Camera::is_free() {
	return context_->is_free();
}

bool Camera::update_context(Point3 vpos) {
	vpos_ = vpos;
	if (free_override_) {
		context_ = &free_context_;
		return false;
	}
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
	std::string label = context_->area_name();
	if (active_label_ != label) {
		active_label_ = label;
		return true;
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

void Camera::handle_free_cam_input(GameState* state) {
	double* tilt_ptr = nullptr;
	double* rot_ptr = nullptr;
	if (context_->is_free()) {
		tilt_ptr = context_->get_tilt_ptr();
		rot_ptr = context_->get_rot_ptr();
	} else if (free_override_) {
		tilt_ptr = free_context_.get_tilt_ptr();
		rot_ptr = free_context_.get_rot_ptr();
	} else {
		return;
	}
	if (state->key_pressed(GLFW_KEY_S)) {
		*tilt_ptr = std::min(*tilt_ptr + FREE_CAM_TILT_SPEED, MAX_CAM_TILT);
	} else if (state->key_pressed(GLFW_KEY_W)) {
		*tilt_ptr = std::max(*tilt_ptr - FREE_CAM_TILT_SPEED, MIN_CAM_TILT);
	}
	if (state->key_pressed(GLFW_KEY_A)) {
		*rot_ptr += FREE_CAM_ROT_SPEED;
		if (*rot_ptr >= TWO_PI) {
			*rot_ptr -= TWO_PI;
		}
	} else if (state->key_pressed(GLFW_KEY_D)) {
		*rot_ptr -= FREE_CAM_ROT_SPEED;
		if (*rot_ptr <= -TWO_PI) {
			*rot_ptr += TWO_PI;
		}
	}
	if (state->key_pressed(GLFW_KEY_E)) {
		*tilt_ptr = DEFAULT_CAM_TILT;
		*rot_ptr = DEFAULT_CAM_ROTATION;
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
