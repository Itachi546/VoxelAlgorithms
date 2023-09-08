#pragma once

#include "voxel-renderer.h"
#include "../voxel-chunk.h"

class Raymarcher : public VoxelRenderer {

public:
	Raymarcher(VoxelChunk* chunk);
	
	void initialize() override;

	void render(OrbitCamera* camera, unsigned int globalUBO) override;

	void destroy() override;

private:
	VoxelChunk* mChunk;
	uint32_t mShader;

	int mStepCount = 64;
};