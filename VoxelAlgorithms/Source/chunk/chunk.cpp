#include "chunk.h"

#include "build-density.h"
#include "../gl.h"

#include "../voxel-algorithm/voxel-generator.h"

Chunk::Chunk(const glm::ivec3& chunkId, uint32_t chunkSize) : mChunkId(chunkId)
{
	gl::TextureParams params;
	gl::initializeTextureParams(params);

	// Generate additional one voxel at the boundary
	mNumVoxel = chunkSize + 1;
	params.wrapMode = GL_CLAMP_TO_EDGE;
	params.width = mNumVoxel;
	params.height = mNumVoxel;
	params.depth = mNumVoxel;
	params.format = GL_RED;
	params.internalFormat = GL_R32F;
	params.type = GL_FLOAT;
	params.target = GL_TEXTURE_3D;

	mDensityTexture = gl::createTexture(params);

	// We generate the vertices that lies at edge 0, 3 or 8 only so that is the maximum
	// of 3 vertex for each voxel each having 6 floats
	const int totalVoxel = mNumVoxel * mNumVoxel * mNumVoxel;
	const int vertexSize = totalVoxel * 18 * sizeof(float);
	// Each voxel can have upto 5 triangle that makes upto 15 possible indices
	const int indexSize = totalVoxel * sizeof(uint32_t) * 15;
	mMesh.create(vertexSize, indexSize);
}

void Chunk::generate(DensityBuilder* builder, VoxelGenerator* generator)
{
	builder->generate(mDensityTexture, mNumVoxel, mNumVoxel, mNumVoxel);

	generator->generate(&mMesh, mDensityTexture, mNumVoxel);
}

void Chunk::draw()
{
	glm::mat4 M = glm::mat4(1.0f);
	glUniformMatrix4fv(0, 1, GL_FALSE, &M[0][0]);
	mMesh.draw();
}

void Chunk::destroy()
{
	gl::destroyTexture(mDensityTexture);
	gl::destroyBuffer(mMesh.vertexBuffer);
	gl::destroyBuffer(mMesh.indexBuffer);
}
