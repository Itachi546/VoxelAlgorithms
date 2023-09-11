#include "chunk-manager.h"

#include "build-density.h"
#include "../voxel-algorithm/marchingcube.h"

void ChunkManager::initialize()
{
	TerrainResourceManager::getInstance()->initialize(kMaxChunkCount, kNumVoxel);

	// Initialize density builder
	GLuint densityCS = gl::createShader("Assets/SPIRV/build-density.comp.spv");
	mDensityShader = gl::createProgram(&densityCS, 1);
	gl::destroyShader(densityCS);
	DensityParams params = { 1.0f, 1.0f, 0.5f, 0.5f, 5 };
	mDensityBuilder = new DensityBuilder(params, mDensityShader);

	mMeshGenerator = MarchingCube::getInstance();
	mMeshGenerator->initialize(kNumVoxel);

	Chunk chunk{glm::ivec3(0), kNumVoxel};
	chunk.generate(mDensityBuilder, mMeshGenerator);
	mChunkList.push_back(chunk);
/*
	Chunk chunk2{ glm::ivec3(kNumVoxel, 0, 0), kNumVoxel };
	chunk2.generate(mDensityBuilder, mMeshGenerator);
	mChunkList.push_back(chunk2);
	*/
}

void ChunkManager::update(float dt)
{
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
