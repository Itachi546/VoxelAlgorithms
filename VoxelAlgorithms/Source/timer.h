#pragma once

#include <chrono>

class Timer {
public:
	static Timer* getInstance() {
		static Timer* timer = new Timer();
		return timer;
	}

	void reset() {
		mLastTime = std::chrono::high_resolution_clock::now();
		mElapsedTime = 0.0;
	}

	void initialize() {
		mLastTime = std::chrono::high_resolution_clock::now();
		mElapsedTime = 0.0;
	}

	// Return frameTime in second
	double tick() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double, std::milli> deltaMs = currentTime - mLastTime;

		mDt = std::chrono::duration_cast<std::chrono::milliseconds>(deltaMs).count() / 1000.0;
		mElapsedTime += mDt;

		mLastTime = currentTime;
		return mDt;
	}

	double getDeltaSeconds() { return mDt; }
	double getElapsedSeconds() { return mElapsedTime; }

private:
	Timer() = default;

	std::chrono::high_resolution_clock::time_point mLastTime = {};
	double mDt = 0.016f;
	double mElapsedTime = 0.0f;

};