#include "chunk-manager.h"

#include "build-density.h"
#include "../voxel-algorithm/marchingcube.h"
#include "../camera.h"

void ChunkManager::initialize(Camera* camera, DensityBuilder* densityBuilder, VoxelGenerator* meshGenerator, uint32_t numVoxel)
{
	kNumVoxel = numVoxel;
	mCamera = camera;
	mDensityBuilder = densityBuilder;
	mMeshGenerator = meshGenerator;

	TerrainResourceManager* resourceManager = TerrainResourceManager::getInstance();
	resourceManager->initialize(kAllocationSize, numVoxel);

	Chunk chunk{ glm::ivec3(0), numVoxel };
	chunk.generate(densityBuilder, meshGenerator);
	mChunkList.push_back(chunk);
	//initializeLoadingList();
}

void ChunkManager::update(float dt)
{
	//updateLoadingList();
}

void ChunkManager::render()
{
	for (auto& chunk : mChunkList) {
		chunk.draw();
	}
}

void ChunkManager::destroy()
{
	TerrainResourceManager* resourceManager = TerrainResourceManager::getInstance();
	resourceManager->destroy();
}

void ChunkManager::initializeLoadingList()
{
	glm::vec3 cameraPosition = mCamera->getPosition();
	glm::ivec3 currentGrid = glm::ivec3(cameraPosition / float(kNumVoxel));

	for (int y = -kMaxChunkRadius; y <= kMaxChunkRadius; ++y) {
		for (int z = -kMaxChunkRadius; z <= kMaxChunkRadius; ++z) {
			for (int x = -kMaxChunkRadius; x <= kMaxChunkRadius; ++x) {
				mChunkLoadingList.push_back(currentGrid + glm::ivec3(x, y, z));
			}
		}
	}
}

void ChunkManager::updateLoadingList()
{
	const uint32_t maxChunkBuildPerFrame = 3;

	for (uint32_t i = 0; i < maxChunkBuildPerFrame; ++i) {
		if (mChunkLoadingList.size() == 0)  break;
		glm::ivec3 chunkToLoad = mChunkLoadingList.back();
		mChunkLoadingList.pop_back();
		Chunk chunk{ chunkToLoad * int(kNumVoxel), kNumVoxel };
		chunk.generate(mDensityBuilder, mMeshGenerator);
		mChunkList.push_back(chunk);
	}
}