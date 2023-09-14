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
class Camera;
class TerrainResourceManager;

class ChunkManager {

public:
	ChunkManager() = default;

	// maxChunkCount
	void initialize(Camera* camera, DensityBuilder* densityBuilder, VoxelGenerator* meshGenerator, uint32_t numVoxel);

	void update(float dt);

	void render();

	void destroy();

private:

	const uint32_t kAllocationSize = 1;

	std::vector<Chunk> mChunkList;

	std::vector<glm::ivec3> mChunkLoadingList;

	const int kMaxChunkRadius = 1;
	Camera* mCamera;

	DensityBuilder* mDensityBuilder;
	VoxelGenerator* mMeshGenerator;

	uint32_t kNumVoxel = 0;

	void initializeLoadingList();

	void updateLoadingList();
};