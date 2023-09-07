#pragma once

#include <stdint.h>

#include "build-density.h"

class VoxelChunk {

public:
	VoxelChunk(uint32_t chunkSize, uint32_t voxelSize);
	
	void generateDensities(DensityBuilder* builder);

	unsigned int getDensityTexture() const { return mDensityTexture; }

	uint32_t getNumVoxel() const { return mNumVoxel; }

private:
	uint32_t mChunkSize;
	uint32_t mVoxelSize;
	uint32_t mNumVoxel;

	unsigned int mDensityTexture;
};
