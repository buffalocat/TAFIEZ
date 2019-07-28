#include "stdafx.h"
#include "animation.h"

#include "common_constants.h"

Animation::Animation(): frames_ {} {}

Animation::~Animation() {}

bool Animation::update() {
    --frames_;
    return frames_ == 0;
}


LinearAnimation::LinearAnimation(Point3 d): PositionalAnimation(), d_ {d} {
    frames_ = HORIZONTAL_MOVEMENT_FRAMES - 1;
}

LinearAnimation::~LinearAnimation() {}

FPoint3 LinearAnimation::dpos() {
    return (-(float)frames_/HORIZONTAL_MOVEMENT_FRAMES)*FPoint3{d_};
}

Point3 LinearAnimation::shift_pos(Point3 p) {
    return p + d_;
}

std::unique_ptr<PositionalAnimation> LinearAnimation::duplicate() {
    return std::make_unique<LinearAnimation>(*this);
}


const double GATE_INTERPOLATION[] = {.96, .9, .8, .6, 0.0};

GateTransitionAnimation::GateTransitionAnimation(bool state): Animation(), state_ {state} {
    frames_ = SWITCH_RESPONSE_FRAMES - 1;
}

GateTransitionAnimation::~GateTransitionAnimation() {}

double GateTransitionAnimation::height() { 
    if (state_) {
        return 1.0 - GATE_INTERPOLATION[4 - frames_];
    } else {
        return GATE_INTERPOLATION[4 - frames_];
    }
}
