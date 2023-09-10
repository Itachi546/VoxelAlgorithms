#pragma once

#include "glm-includes.h"

#include <unordered_map>

static const uint32_t MAX_KEYS = 512;
struct KeyboardState {
	bool keys[MAX_KEYS];
};

struct MouseState {
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 delta{ 0.0f, 0.0f };
	glm::vec2 deltaNormalized{ 0.0f, 0.0f };

	float scrollWheelX = 0.0f;
	float scrollWheelY = 0.0f;

	float lastScrollX = 0.0f;
	float lastScrollY = 0.0f;

	bool isScrolled = false;
	bool wasScrolled = false;
};

class Input {

public:
	static Input* getInstance() {
		static Input* input = new Input();
		return input;
	}

	void initialize() {
	}

	void setKeyState(int key, bool action) {
		mCurrentKeys.keys[key] = action;
	}

	void setScrollOffset(float xOffset, float yOffset) {
		mMouseState.scrollWheelX = xOffset;
		mMouseState.scrollWheelY = yOffset;
		mMouseState.isScrolled = true;
	}

	glm::vec2 getScrollOffset() const {
		return { mMouseState.lastScrollX, mMouseState.lastScrollY };
	}

	void update() {
		std::memcpy(mLastKeys.keys, mCurrentKeys.keys, sizeof(bool) * MAX_KEYS);

		mMouseState.lastScrollX = mMouseState.scrollWheelX;
		mMouseState.lastScrollY = mMouseState.scrollWheelY;

		mMouseState.scrollWheelX = 0.0f;
		mMouseState.scrollWheelY = 0.0f;

		mMouseState.wasScrolled = mMouseState.isScrolled;
		mMouseState.isScrolled = false;
	}

	void setMousePosition(float x, float y, uint32_t width, uint32_t height) {
		glm::vec2 lastPos = mMouseState.position;
		glm::vec2 delta = { x - lastPos.x, y - lastPos.y };
		mMouseState.deltaNormalized = { delta.x / float(width), delta.y / float(height) };
		mMouseState.delta = delta;

		mMouseState.position = { x, y };
	}

	bool isScrolled() {
		return mMouseState.wasScrolled;
	}

	bool isKeyDown(int key) const {
		return mCurrentKeys.keys[key];
	}

	bool wasKeyPressed(int key) const {
		return mCurrentKeys.keys[key] && !mLastKeys.keys[key];
	}

	glm::vec2 getMousePos() const {
		return mMouseState.position;
	}

	glm::vec2 getMouseDelta() const {
		return mMouseState.delta;
	}

	glm::vec2 getMouseDeltaNormalized() const {
		return mMouseState.deltaNormalized;
	}
private:
	Input() {
		for (int i = 0; i < MAX_KEYS; ++i) {
			mCurrentKeys.keys[i] = false;
			mLastKeys.keys[i] = false;
		}
	}
	KeyboardState mCurrentKeys;
	KeyboardState mLastKeys;
	MouseState mMouseState;
};