#include "chunk-manager.h"

#include "build-density.h"
#include "../voxel-algorithm/marchingcube.h"
#include "../camera.h"

void ChunkManager::initialize(Camera* camera)
{
	mCamera = camera;

	TerrainResourceManager::getInstance()->initialize(kAllocationSize, kNumVoxel);

	// Initialize density builder
	GLuint densityCS = gl::createShader("Assets/SPIRV/build-density.comp.spv");
	mDensityShader = gl::createProgram(&densityCS, 1);
	gl::destroyShader(densityCS);
	DensityParams params = { 1.0f, 1.0f, 0.5f, 0.5f, 5 };
	mDensityBuilder = new DensityBuilder(params, mDensityShader);

	mMeshGenerator = MarchingCube::getInstance();
	mMeshGenerator->initialize(kNumVoxel);

	initializeLoadingList();
}

void ChunkManager::update(float dt)
{
	updateLoadingList();
}

void ChunkManager::render()
{
	for (auto& chunk : mChunkList) {
		chunk.draw();
	}
}

void ChunkManager::destroy()
{
	gl::destroyProgram(mDensityShader);

	mMeshGenerator->destroy();
	TerrainResourceManager::getInstance()->destroy();
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