#pragma once

#include <stdint.h>
#include "../glm-includes.h"
#include "terrain-resource-manager.h"

class DensityBuilder;
class VoxelGenerator;
struct GpuMesh;

class Chunk {

public:
	Chunk(const glm::ivec3& chunkId, uint32_t chunkSize);

	void generate(DensityBuilder* builder, VoxelGenerator* generator);

	void draw();
	
	unsigned int getDensityTexture() const { return mDensityTexture.texture; }

	uint32_t getNumVoxel() const { return mNumVoxel; }

	glm::ivec3 getChunkId()const { return mChunkId; }

	void destroy();

private:
	glm::ivec3 mChunkId;

	uint32_t mNumVoxel;

	MeshResourceHandle mMesh;
	TextureResourceHandle mDensityTexture;

	static uint32_t totalEmptyChunk;
	static uint32_t totalNonEmptyChunk;
};
