#pragma once

#include <stdint.h>

class Camera;
class Chunk;
struct GpuMesh;

class VoxelGenerator {

public:
	virtual void initialize(uint32_t numVoxel) = 0;

	virtual void generate(GpuMesh* mesh, uint32_t densityTexture, uint32_t numVoxel) = 0;

	virtual void destroy() = 0;

	virtual ~VoxelGenerator() = default;

protected:
	VoxelGenerator() = default;

	uint32_t mWidth;
	uint32_t mHeight;
};