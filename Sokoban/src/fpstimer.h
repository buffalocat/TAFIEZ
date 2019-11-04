#pragma once

#include <chrono>

using time_point = std::chrono::steady_clock::time_point;
using ns = std::chrono::nanoseconds;

class FPSTimer {
public:
	FPSTimer(unsigned int fps);
	~FPSTimer();
	void wait_for_frame_end();

private:
	time_point frame_start_;
	ns nspf_;
	double offset_{0};
};