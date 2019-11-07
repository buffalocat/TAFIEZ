#include "stdafx.h"
#include "fpstimer.h"

#include <thread>

FPSTimer::FPSTimer(unsigned int fps) :
	frame_start_{ std::chrono::high_resolution_clock::now() },
	nspf_{ std::chrono::nanoseconds{1000000000 / fps} } {}

FPSTimer::~FPSTimer() {}

void FPSTimer::wait_for_frame_end() {
	auto frame_end = std::chrono::high_resolution_clock::now();
	auto frame_time = frame_end - frame_start_;
	auto sleep_time = nspf_ - frame_time - std::chrono::duration<double, std::nano>(offset_);
	std::this_thread::sleep_for(sleep_time);
	frame_start_ += nspf_;
	auto cur_time = std::chrono::high_resolution_clock::now();
	auto oversleep = cur_time - frame_start_;
	// If we slept a frame later than we should have, something bad happened; reset
	if (oversleep > nspf_) {
		oversleep = nspf_;
		frame_start_ = cur_time - nspf_;
	}
	offset_ = 0.8 * offset_ + 0.2 * std::chrono::duration_cast<ns>(oversleep).count();
}