#pragma once

#include "resource-pool.h"

#include "../mesh.h"

#include <vector>

struct MeshResourceHandle {
	uint32_t id;
	GpuMesh* mesh;
};

using GpuTextureHandle = uint32_t;
struct TextureResourceHandle {
	uint32_t id;
	GpuTextureHandle texture;
};

class TerrainResourceManager {

public:
	static TerrainResourceManager* getInstance() {
		static TerrainResourceManager* resourceManager = new TerrainResourceManager();
		return resourceManager;
	}

	void initialize(uint32_t maxAllocation, uint32_t mNumVoxel);

	MeshResourceHandle getMesh();

	void releaseMesh(MeshResourceHandle handle);

	// Texture are lazy initialized as almost all might be used in future
	// Mesh are different case, we may have empty density texture but allocating
	// mesh is waste
	TextureResourceHandle getTexture();

	void releaseTexture(TextureResourceHandle texture);

	void destroy();

private:
	TerrainResourceManager() = default;

	ResourcePool<GpuMesh> mMeshPool;
	ResourcePool<GpuTextureHandle> mTexturePool;

	uint32_t mNumVoxel = 32;
};