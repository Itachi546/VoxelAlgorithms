#pragma once

#include <stdint.h>
#include "../glm-includes.h"
#include "../mesh.h"

#include "../voxel-algorithm/voxel-generator.h"

class DensityBuilder;

class Chunk {

public:
	Chunk(const glm::ivec3& chunkId, uint32_t chunkSize);

	void generate(DensityBuilder* builder, VoxelGenerator* generator);

	void draw();
	
	unsigned int getDensityTexture() const { return mDensityTexture; }

	uint32_t getNumVoxel() const { return mNumVoxel; }

	glm::ivec3 getChunkId()const { return mChunkId; }

	void destroy();

private:
	glm::ivec3 mChunkId;

	uint32_t mNumVoxel;

	GpuMesh mMesh;

	unsigned int mDensityTexture;
};
