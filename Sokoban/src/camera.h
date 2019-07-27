#ifndef CAMERA_H
#define CAMERA_H

#include "point.h"

class RoomMap;
class MapFileI;
class MapFileO;

class CameraContext {
public:
    CameraContext(std::string label, int x, int y, int w, int h, int priority);
    virtual ~CameraContext() = 0;
    virtual bool is_null();
    virtual FPoint3 center(FPoint3);
    virtual float radius(FPoint3);
    virtual float tilt(FPoint3);
    virtual float rotation(FPoint3);
    virtual void serialize(MapFileO& file);

	std::string label_;

	int x_;
    int y_;
    int w_;
    int h_;
    int priority_;
};

class FreeCameraContext: public CameraContext {
public:
    FreeCameraContext(std::string label, int x, int y, int w, int h, int priority, float radius, float tilt, float rotation);
    ~FreeCameraContext();
    FPoint3 center(FPoint3);
    float radius(FPoint3);
    float tilt(FPoint3);
    float rotation(FPoint3);
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

	void change_rotation(float dr);

    float rad_;
    float tilt_;
    float rot_;
};

class FixedCameraContext: public CameraContext {
public:
    FixedCameraContext(std::string label, int x, int y, int w, int h, int priority, float radius, float tilt, float rotation, FPoint3 center);
    ~FixedCameraContext();
    FPoint3 center(FPoint3);
    float radius(FPoint3);
    float tilt(FPoint3);
    float rotation(FPoint3);
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

    float rad_;
    float tilt_;
    float rot_;
    FPoint3 center_;
};

class ClampedCameraContext: public CameraContext {
public:
    ClampedCameraContext(std::string label, int x, int y, int w, int h, int priority, float radius, float tilt, int xpad, int ypad);
    ~ClampedCameraContext();
    FPoint3 center(FPoint3);
    float radius(FPoint3);
    float tilt(FPoint3);
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);

    float rad_;
    float tilt_;
    int xpad_;
    int ypad_;
};

class NullCameraContext: public CameraContext {
public:
    NullCameraContext(std::string label, int x, int y, int w, int h, int priority);
    ~NullCameraContext();
    bool is_null();
    void serialize(MapFileO& file);
    static CameraContext* deserialize(MapFileI& file);
};

class Camera {
public:
    Camera(int w, int h);
    void serialize(MapFileO& file);
    void update();
    void set_target(Point3, FPoint3);
    void set_current_pos(Point3);
    float get_radius();
    FPoint3 get_pos();
    float get_tilt();
    float get_rotation();
	void change_rotation(float);

    void push_context(std::unique_ptr<CameraContext>);
	void remove_context(CameraContext*);
	std::vector<std::unique_ptr<CameraContext>>& loaded_contexts();

private:
    int width_;
    int height_;
    FreeCameraContext default_context_;
    CameraContext* context_;
    std::vector<std::unique_ptr<CameraContext>> loaded_contexts_;
    std::vector<std::vector<CameraContext*>> context_map_;
    FPoint3 target_pos_;
    FPoint3 cur_pos_;
    float target_rad_;
    float cur_rad_;
    float target_tilt_;
    float cur_tilt_;
    float target_rot_;
    float cur_rot_;
};

float damp_avg(float target, float cur);


#endif // CAMERA_H
