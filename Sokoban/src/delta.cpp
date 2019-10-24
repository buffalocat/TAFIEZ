#include "stdafx.h"
#include "delta.h"


Delta::~Delta() {}

DeltaFrame::DeltaFrame() {}

DeltaFrame::~DeltaFrame() {}

void DeltaFrame::revert() {
	for (auto it = deltas_.rbegin(); it != deltas_.rend(); ++it) {
		(**it).revert();
	}
}

void DeltaFrame::push(std::unique_ptr<Delta> delta) {
	deltas_.push_back(std::move(delta));
	changed_ = true;
}

bool DeltaFrame::trivial() {
	return deltas_.empty();
}

void DeltaFrame::reset_changed() {
	changed_ = false;
}

bool DeltaFrame::changed() {
	return changed_;
}


UndoStack::UndoStack(unsigned int max_depth) : frames_{}, max_depth_{ max_depth }, size_{ 0 } {}

UndoStack::~UndoStack() {}

void UndoStack::push(std::unique_ptr<DeltaFrame> delta_frame) {
	if (!delta_frame->trivial()) {
		if (size_ == max_depth_) {
			frames_.pop_front();
		} else {
			++size_;
		}
		frames_.push_back(std::move(delta_frame));
	}
}

bool UndoStack::non_empty() {
	return size_ > 0;
}

void UndoStack::pop() {
	frames_.back()->revert();
	frames_.pop_back();
	--size_;
}

void UndoStack::reset() {
	frames_.clear();
	size_ = 0;
}
