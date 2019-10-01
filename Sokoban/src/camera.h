#ifndef CAMERA_H
#define CAMERA_H

#include "point.h"

class DeltaFrame;
class RoomMap;
class MapFileI;
class MapFileO;

namespace CAM_FLAGS {
	enum {
		DEFAULT = 0,
		// General information
		NULL_AREA = 1 << 1,
		NAMED_AREA = 1 << 2,
		HAS_NULL_CHILD = 1 << 3,
		FREE_CAM = 1 << 4,
		// How to compute Visible rect from Active rect
		PAD_UNIFORM = 1 << 5,
		PAD_XY = 1 << 6,
		PAD_ALL = 1 << 7,
		VIS_AUTO = PAD_UNIFORM | PAD_XY | PAD_ALL,
		// How to compute Radius from Visible rect
		RAD_SMALLER = 1 << 8,
		RAD_LARGER = 1 << 9,
		RAD_CLAMP_X = 1 << 10,
		RAD_CLAMP_Y = 1 << 11,
		RAD_DEFAULT = 1 << 12,
		RAD_AUTO = RAD_SMALLER | RAD_LARGER | RAD_CLAMP_X | RAD_CLAMP_Y | RAD_DEFAULT,
		// How to compute Center using Visible rect
		POS_CLAMP_X = 1 << 14,
		POS_FIX_X = 1 << 15,
		POS_CLAMP_Y = 1 << 16,
		POS_FIX_Y = 1 << 17,
		CENTER_USED = POS_CLAMP_X | POS_FIX_X | POS_CLAMP_Y | POS_FIX_Y,
		// Use custom tilt? rotation?
		TILT_CUSTOM = 1 << 18,
		ROT_CUSTOM = 1 << 19,
	};
};

class CamRectCalculator {
public:
	CamRectCalculator(double tilt);
	~CamRectCalculator();
	void set_tilt(double tilt);
	double compute_radius(FloatRect vis, unsigned int flags);
	FloatRect compute_center_from_vis_rad(FloatRect r, double rad);

private:
	double tilt_;
	double hor_excess_factor_;
	double upper_excess_factor_;
	double lower_excess_factor_;
};

class CameraContext {
public:
    CameraContext(IntRect rect, int priority, std::string label);
    ~CameraContext();
	virtual bool is_null();
	virtual bool has_null_child();
	virtual bool is_free();

	virtual void serialize(MapFileO& file);
    virtual FPoint3 center(FPoint3);
    virtual double radius(FPoint3);
    virtual double tilt(FPoint3);
    virtual double rotation(FPoint3);
	virtual std::string area_name();

	virtual void shift_by(Point3 d, int width, int height);
	virtual void extend_by(Point3 d, int width, int height);

	virtual double* get_tilt_ptr();
	virtual double* get_rot_ptr();

	IntRect rect_;
	int priority_;
	std::string label_;
};

struct Padding {
	int left, right, top, bottom;
};

class GeneralCameraContext: public CameraContext {
public:
    GeneralCameraContext(IntRect rect, int priority, std::string label, unsigned int flags);
    ~GeneralCameraContext();
	void serialize(MapFileO& file);
	static std::unique_ptr<CameraContext> deserialize(MapFileI& file);

	bool is_null();
	bool has_null_child();
	virtual bool is_free();
	virtual bool can_override_free();

	std::string area_name();
    FPoint3 center(FPoint3);
    double radius(FPoint3);
    double tilt(FPoint3);
	double rotation(FPoint3);

	void shift_by(Point3 d, int width, int height);

	double* get_tilt_ptr();
	double* get_rot_ptr();

private:
	unsigned int flags_;
	double rad_;
	double tilt_;
	double rot_;
	FloatRect visible_;
	Padding pad_;
	FloatRect center_;

	friend class GeneralContextData; // A class to help create/edit contexts
};

class DependentNullCameraContext: public CameraContext {
public:
    DependentNullCameraContext(IntRect rect, int priority, std::string label);
    ~DependentNullCameraContext();
    bool is_null();
};

class Camera {
public:
    Camera(int w, int h);
    void serialize(MapFileO& file);
    void update();
	bool update_context(Point3 vpos);
    void set_target(FPoint3 rpos);
    void set_current_to_target();
    double get_radius();
    FPoint3 get_pos();
    double get_tilt();
    double get_rotation();

	bool update_label();

    void push_context(std::unique_ptr<CameraContext>);
	void remove_context(CameraContext*);
	std::vector<std::unique_ptr<CameraContext>>& loaded_contexts();

	void shift_by(Point3 d);
	void extend_by(Point3 d);

	void handle_free_cam_input(GLFWwindow* window);

	std::string active_label_;

private:
	int label_display_cooldown_;
	int width_;
	int height_;
    GeneralCameraContext default_context_;
	GeneralCameraContext free_context_;
    CameraContext* context_;
    std::vector<std::unique_ptr<CameraContext>> loaded_contexts_;
    std::vector<std::vector<CameraContext*>> context_map_;
    FPoint3 target_pos_;
    FPoint3 cur_pos_;
    double target_rad_;
    double cur_rad_;
    double target_tilt_;
    double cur_tilt_;
    double target_rot_;
    double cur_rot_;
	bool free_override_ = false;

	friend class LabelChangeDelta;
};

double damp_avg(double target, double cur);


#endif // CAMERA_H
