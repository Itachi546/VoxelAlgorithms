#pragma once

#include "voxel-renderer.h"
#include "../voxel-chunk.h"

#include "../gl.h"

class MarchingCube : public VoxelRenderer
{
public:
	MarchingCube(VoxelChunk* chunk, unsigned int drawShader);

	void initialize() override;

	void render(Camera* camera, unsigned int globalUBO) override;

	void destroy() override;

private:
	VoxelChunk* mChunk;

	uint32_t mSplatBuffer;
	uint32_t mTriLUTBuffer;
	uint32_t mVertexBuffer;
	uint32_t mIndexBuffer;

	uint32_t mCountBuffer;
	uint32_t* mCountBufferPtr;
	uint32_t mTriangleCount;
	uint32_t mVertexCount;

	uint32_t mListVerticesShader;
	uint32_t mListIndicesShader;
	uint32_t mDrawShader;

	GLsync mSync;
	
	void generateMesh();

	void generateVertices();
	void generateIndices();
};
