#pragma once

#include <stdint.h>

class OrbitCamera;

class VoxelRenderer {

public:
	virtual void initialize() = 0;
	virtual void render(OrbitCamera* camera) = 0;
	virtual void destroy() = 0;

	void setSize(uint32_t width, uint32_t height) {
		mWidth = width;
		mHeight = height;
	}

	virtual ~VoxelRenderer() = default;

protected:
	uint32_t mWidth;
	uint32_t mHeight;
};