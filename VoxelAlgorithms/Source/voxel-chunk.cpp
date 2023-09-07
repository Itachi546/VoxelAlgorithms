#include "voxel-chunk.h"

#include "gl.h"

VoxelChunk::VoxelChunk(uint32_t chunkSize, uint32_t voxelSize) : mVoxelSize(voxelSize), mChunkSize(chunkSize)
{
	gl::TextureParams params;
	gl::initializeTextureParams(params);

	mNumVoxel = (chunkSize / voxelSize) + 1;
	params.wrapMode = GL_CLAMP_TO_EDGE;
	params.width = mNumVoxel;
	params.height = mNumVoxel;
	params.depth = mNumVoxel;
	params.format = GL_RED;
	params.internalFormat = GL_R32F;
	params.type = GL_FLOAT;
	params.target = GL_TEXTURE_3D;

	mDensityTexture = gl::createTexture(params);
}

void VoxelChunk::generateDensities(DensityBuilder* builder)
{
	builder->generate(mDensityTexture, mNumVoxel, mNumVoxel, mNumVoxel);
}
