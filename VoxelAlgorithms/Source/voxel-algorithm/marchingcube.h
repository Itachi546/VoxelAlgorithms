#pragma once

#include "voxel-generator.h"

#include "../gl.h"

class MarchingCube : public VoxelGenerator
{
public:
	static MarchingCube* getInstance() {
		static MarchingCube* mc = new MarchingCube();
		return mc;
	}

	void initialize(uint32_t numVoxel) override;

	void generate(GpuMesh* mesh, uint32_t densityTexture, uint32_t numVoxel) override;

	void destroy() override;

	float timeToGenerateVertices = 0.0f;
	float timeToGenerateIndices = 0.0f;
private:
	MarchingCube() = default;

	uint32_t mSplatBuffer;
	uint32_t mTriLUTBuffer;

	uint32_t mCountBuffer;
	uint32_t* mCountBufferPtr;

	uint32_t mListVerticesShader;
	uint32_t mListIndicesShader;

	GLsync mSync;
	unsigned int mTimerQuery[16];
	
	void generateVertices(GpuMesh* mesh, uint32_t densityTexture, uint32_t numVoxel);
	void generateIndices(GpuMesh* mesh, uint32_t densityTexture, uint32_t numVoxel);
};
