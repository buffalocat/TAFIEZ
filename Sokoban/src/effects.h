#pragma once

#include "point.h"

class GameObject;
class GraphicsManager;

struct FallTrail {
    Point3 base;
    int height;
    int opacity;
    int color;
};

class Effects {
public:
    Effects();
    ~Effects();
    void sort_by_distance(double angle);
    void update();
    void draw(GraphicsManager*);
    void push_trail(GameObject*, int height, int drop);

private:
    std::vector<FallTrail> trails_;
};