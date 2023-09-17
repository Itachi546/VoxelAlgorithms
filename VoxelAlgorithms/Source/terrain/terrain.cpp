#include "terrain.h"

#include "chunk-manager.h"
#include "build-density.h"
#include "terrain-physics.h"
#include "../voxel-algorithm/marchingcube.h"

void Terrain::initialize(Camera* camera)
{
	// Initialize density builder
	DensityParams params = { 1.0f, 0.008f, 2.0f, 0.5f, 7 };
	mDensityBuilder = new DensityBuilder(params);

	mMeshGenerator = MarchingCube::getInstance();
	mMeshGenerator->initialize(kNumVoxel);

	mChunkManager = new ChunkManager();
	mChunkManager->initialize(camera, mDensityBuilder, mMeshGenerator, kNumVoxel);

	mPhysicsManager = new TerrainPhysics();
	mPhysicsManager->initialize(mDensityBuilder->densityParamUB);

}

void Terrain::update(float dt)
{
	mChunkManager->update(dt);
	mPhysicsManager->update(dt);
}

void Terrain::render()
{
	mChunkManager->render();
}

void Terrain::destroy()
{
	mChunkManager->destroy();
	delete mChunkManager;

	mDensityBuilder->destroy();
	delete mDensityBuilder;

	mMeshGenerator->destroy();
}
