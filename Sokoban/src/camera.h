#ifndef CAMERA_H
#define CAMERA_H

#include "point.h"

class DeltaFrame;
class RoomMap;
class MapFileI;
class MapFileO;
class GraphicsManager;

class CameraContext {
public:
    CameraContext(std::string label, IntRect rect, int priority, bool named_area, bool null_child);
    virtual ~CameraContext() = 0;
    virtual bool is_null();
    virtual FPoint3 center(FPoint3);
    virtual double radius(FPoint3);
    virtual double tilt(FPoint3);
    virtual double rotation(FPoint3);
    virtual void serialize(MapFileO& file) = 0;

	virtual void shift_by(Point3 d, int width, int height);
	virtual void extend_by(Point3 d, int width, int height);

	std::string label_;

	IntRect rect_;
    int priority_;
	bool named_area_;
	bool null_child_;
};

class ClampedCameraContext: public CameraContext {
public:
    ClampedCameraContext(std::string label, IntRect rect, int priority, bool named_area, bool null_child,
		double radius, double tilt, FloatRect center);
    ~ClampedCameraContext();
    FPoint3 center(FPoint3);
    double radius(FPoint3);
    double tilt(FPoint3);
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

	void shift_by(Point3 d, int width, int height);

    double rad_;
    double tilt_;
	FloatRect center_;
};

class NullCameraContext: public CameraContext {
public:
    NullCameraContext(std::string label, IntRect rect, int priority, bool named_area, bool independent);
    ~NullCameraContext();
    bool is_null();
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

	bool independent_;
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

	void update_label(GraphicsManager* gfx);

    void push_context(std::unique_ptr<CameraContext>);
	void remove_context(CameraContext*);
	std::vector<std::unique_ptr<CameraContext>>& loaded_contexts();

	void shift_by(Point3 d);
	void extend_by(Point3 d);

private:
	std::string active_label_;
	int label_display_cooldown_;
	int width_;
	int height_;
    ClampedCameraContext default_context_;
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

	friend class LabelChangeDelta;
};

double damp_avg(double target, double cur);


#endif // CAMERA_H
