#include "chunk.h"

#include "build-density.h"
#include "../voxel-algorithm/voxel-generator.h"

#include "../gl.h"
#include "../mesh.h"


Chunk::Chunk(const glm::ivec3& chunkId, uint32_t chunkSize) : mChunkId(chunkId),
  mNumVoxel(chunkSize)
{
	mMesh = { INVALID_RESOURCE_HANDLE, nullptr };
	mDensityTexture = { INVALID_RESOURCE_HANDLE, ~0u };
}

void Chunk::generate(DensityBuilder* builder, VoxelGenerator* generator)
{
	TerrainResourceManager* resourceManager = TerrainResourceManager::getInstance();

	TextureResourceHandle texture = resourceManager->getTexture();
	builder->generate(texture.texture, mNumVoxel + 1, mNumVoxel + 1, mNumVoxel + 1, mChunkId);

	MeshResourceHandle mesh = resourceManager->getMesh();
	generator->generate(mesh.mesh, texture.texture, mNumVoxel);

	// If no vertices are generated for this chunk, then we
	// can release the mesh and texture resource for use 
	// for other chunk
	if (mesh.mesh->vertexCount == 0) {
		resourceManager->releaseMesh(mesh);
		resourceManager->releaseTexture(texture);
	}
	else {
		mMesh = mesh;
		mDensityTexture = texture;
	}
}

void Chunk::draw()
{
	if (mMesh.mesh != nullptr) {
		glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(mChunkId));
		glUniformMatrix4fv(0, 1, GL_FALSE, &M[0][0]);
		mMesh.mesh->draw();
	}
}

void Chunk::destroy()
{
	TerrainResourceManager* resourceManager = TerrainResourceManager::getInstance();
	if (mMesh.id != INVALID_RESOURCE_HANDLE)
		resourceManager->releaseMesh(mMesh);
	if (mDensityTexture.id != INVALID_RESOURCE_HANDLE)
		resourceManager->releaseTexture(mDensityTexture);
}
