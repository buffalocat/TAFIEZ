#include "stdafx.h"
#include "color_constants.h"

bool Color4::operator==(Color4& c) {
    return r == c.r && g == c.g && b == c.b && a == c.a;
}