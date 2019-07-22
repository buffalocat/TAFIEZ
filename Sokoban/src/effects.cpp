#include "stdafx.h"
#include "effects.h"

#include "graphicsmanager.h"
#include "texture_constants.h"
#include "gameobject.h"

Effects::Effects() : trails_{} {}

Effects::~Effects() {}

const unsigned int FALL_TRAIL_OPACITY = 8;
const float MAX_OPACITY = 10.0;
const float MAX_WIDTH = 16.0;

void Effects::sort_by_distance(float angle) {}

void Effects::update() {
    for (auto& trail : trails_) {
        --trail.opacity;
    }
}

void Effects::draw(GraphicsManager* gfx) {
    for (auto& trail : trails_) {
        glm::vec4 color = COLOR_VECTORS[trail.color];
        color.w = trail.opacity/MAX_OPACITY;
        Point3 base = trail.base;
		gfx->cube.push_instance(glm::vec3(base.x, base.y, base.z - trail.height/2.0f + 0.5f),
			glm::vec3(trail.opacity / MAX_WIDTH, trail.height, trail.opacity / MAX_WIDTH),
			BlockTexture::Blank, color);
    }
    trails_.erase(std::remove_if(trails_.begin(), trails_.end(), [](FallTrail t) {return t.opacity == 0;}), trails_.end());
}

void Effects::push_trail(GameObject* block, int height, int drop) {
    trails_.push_back({block->pos_ - Point3{0,0,drop}, height+drop, FALL_TRAIL_OPACITY, block->color()});
}
