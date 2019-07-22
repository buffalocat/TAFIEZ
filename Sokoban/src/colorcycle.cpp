#include "stdafx.h"
#include "colorcycle.h"

ColorCycle::ColorCycle() : colors_{}, size_{ 0 }, index_{ 0 } {}

ColorCycle::ColorCycle(int color): colors_ {color}, size_ {1}, index_ {0} {}

ColorCycle::ColorCycle(unsigned char* b): colors_ {}, size_ {b[0]}, index_ {b[1]} {
    for (int i = 0; i < size_; ++i) {
        colors_[i] = b[i+2];
    }
}

ColorCycle::~ColorCycle() {}

int ColorCycle::next_color() {
    return colors_[index_];
}

// Swap parent_color with the next color in the cycle
bool ColorCycle::cycle(int* parent_color, bool undo) {
	if (size_) {
		int new_color = colors_[index_];
		colors_[index_] = *parent_color;
		if (!undo) {
			++index_;
			if (index_ == size_) {
				index_ = 0;
			}
		}
		else {
			if (index_ == 0) {
				index_ = size_ - 1;
			}
			else {
				--index_;
			}
		}
		*parent_color = new_color;
		return true;
	}
	return false;
}
