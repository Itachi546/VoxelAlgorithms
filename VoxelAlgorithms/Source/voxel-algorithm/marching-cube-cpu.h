#pragma once

#include "voxel-renderer.h"
#include "../mesh.h"

class VoxelChunk;

class MarchingCubeCpu : public VoxelRenderer {
	public:
	MarchingCubeCpu(VoxelChunk* chunk, uint32_t drawShader);

	void initialize() override;

	void render(OrbitCamera* camera, unsigned int globalUBO) override;

	void destroy() override;

	~MarchingCubeCpu() {}

private:
	VoxelChunk* mChunk;
	uint32_t mDrawShader;

	void generateMesh();
	void generateVertices(std::vector<Vertex>& vertices, std::vector<glm::uvec3>& splatArray);
	void generateIndices(std::vector<uint32_t>& indices, const std::vector<glm::uvec3>& splatArray);

	Mesh mMesh;
};