#include "terrain-resource-manager.h"

#include "../gl.h"

#include <algorithm>

void TerrainResourceManager::initialize(uint32_t maxAllocation, uint32_t numVoxel)
{
	mNumVoxel = numVoxel;
	mMeshPool.initialize(maxAllocation);
	mTexturePool.initialize(maxAllocation);

	// Allocate the size of static vertex and index buffer
	const uint32_t totalVoxel = mNumVoxel * mNumVoxel * mNumVoxel;
	const uint32_t vertexSize = totalVoxel * 18 * sizeof(float);
	// Each voxel can have upto 5 triangle that makes upto 15 possible indices
	const int indexSize = totalVoxel * sizeof(uint32_t) * 15;

	for (uint32_t i = 0; i < maxAllocation; ++i)
	{
		GpuMesh* mesh = static_cast<GpuMesh*>(&mMeshPool.resourceMemory[i]);
		mesh->create(vertexSize, indexSize);

		// Create new texture and add it to resource pool
		gl::TextureParams params;
		gl::initializeTextureParams(params);

		// Generate additional one voxel at the boundary
		params.wrapMode = GL_CLAMP_TO_EDGE;
		params.width = mNumVoxel + 2;
		params.height = mNumVoxel + 2;
		params.depth = mNumVoxel + 2;
		params.format = GL_RED;
		params.internalFormat = GL_R32F;
		params.type = GL_FLOAT;
		params.target = GL_TEXTURE_3D;
		uint32_t densityTexture = gl::createTexture(params);

		GpuTextureHandle* texture = static_cast<GpuTextureHandle*>(&mTexturePool.resourceMemory[i]);
		*texture = densityTexture;
	}

	uint32_t textureSize = (mNumVoxel + 1) * (mNumVoxel + 1) * (mNumVoxel + 1) * 32;

	float perChunkAllocationMB = (vertexSize + indexSize + textureSize) / (1024.0f * 1024.0f);
	std::cout << "Memory Per Chunk Vertex[ " << vertexSize / (1024.0f * 1024.0f) << " ]" <<
		" Indices[ " << indexSize / (1024.0f * 1024.0f) << " ]" <<
		" Texture[ " << textureSize / (1024.0f * 1024.0f) << " ]" << std::endl;

	std::cout << "Total GpuMemory Allocated For Chunk Mesh/Texture Per Chunk: " <<
		 perChunkAllocationMB << " MB" << std::endl;
	std::cout << "Total GpuMemory Allocated For Chunk Mesh/Texture : " <<
		perChunkAllocationMB * maxAllocation << " MB" << std::endl;

}

MeshResourceHandle TerrainResourceManager::getMesh()
{
	uint32_t id = mMeshPool.obtainResource();
	if (id != INVALID_RESOURCE_HANDLE)
		return MeshResourceHandle{ id, mMeshPool.accessResource(id)};
	else
		return MeshResourceHandle{ INVALID_RESOURCE_HANDLE, nullptr };
}

void TerrainResourceManager::releaseMesh(MeshResourceHandle handle)
{
	mMeshPool.releaseResource(handle.id);
}

TextureResourceHandle TerrainResourceManager::getTexture()
{
	uint32_t id = mTexturePool.obtainResource();
	if (id != INVALID_RESOURCE_HANDLE)
		return TextureResourceHandle{ id, *mTexturePool.accessResource(id) };
	else
	{
		std::cerr << "Not enough resource allocated" << std::endl;
		assert(0);
		return TextureResourceHandle{ INVALID_RESOURCE_HANDLE, INVALID_RESOURCE_HANDLE };
	}

}

void TerrainResourceManager::releaseTexture(TextureResourceHandle texture)
{
	mTexturePool.releaseResource(texture.id);
}

void TerrainResourceManager::destroy()
{
	for (uint32_t i = 0; i < mMeshPool.poolSize; ++i) {
		GpuMesh* mesh = mMeshPool.accessResource(i);
		mesh->destroy();

		uint32_t textureId = *mTexturePool.accessResource(i);
		gl::destroyTexture(textureId);
	}
	mTexturePool.destroy();
	mMeshPool.destroy();
}
