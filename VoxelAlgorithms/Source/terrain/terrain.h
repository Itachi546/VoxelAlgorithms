#pragma once

#include <stdint.h>

class Camera;
class ChunkManager;
class DensityBuilder;
class VoxelGenerator;
class TerrainPhysics;

class Terrain {
public:
	static Terrain* getInstance() {
		static Terrain* terrain = new Terrain();
		return terrain;
	}

	void initialize(Camera* camera);

	void update(float dt);

	void render();

	void destroy();

	TerrainPhysics* getPhysicsManager() { return mPhysicsManager; }
private:
	Terrain() = default;

	uint32_t kNumVoxel = 256;

	ChunkManager* mChunkManager = nullptr;
	DensityBuilder* mDensityBuilder = nullptr;
	VoxelGenerator* mMeshGenerator = nullptr;
	TerrainPhysics* mPhysicsManager = nullptr;
};