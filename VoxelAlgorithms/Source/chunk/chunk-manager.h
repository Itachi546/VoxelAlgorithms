#pragma once

#include <stdint.h>

#include "resource-pool.h"
#include "chunk.h"
#include "../mesh.h"


/*
* ChunkManager -----> List<Chunk> mPools
* HashMap<ID, Chunk> mChunkMap
* List<ID> mFreeChunk
*  Chunk -------> GpuMesh | null
* GpuMeshPool
* 
*/

class DensityBuilder;

class ChunkManager {

public:
	static ChunkManager* getInstance() {
		static ChunkManager* manager = new ChunkManager();
		return manager;
	}

	// maxChunkCount
	void initialize();

	void update(float dt);

	void render();

	void destroy();

	uint32_t getNumVoxel() const { return kNumVoxel; }

private:
	ChunkManager() = default;

	const uint32_t kMaxChunkCount = 2;

	std::vector<Chunk> mChunkList;

	std::vector<glm::ivec3> mChunkLoadingList;

	uint32_t kNumVoxel = 32;

	uint32_t mDensityShader = 0;
	DensityBuilder* mDensityBuilder = nullptr;
	VoxelGenerator* mMeshGenerator = nullptr;
};