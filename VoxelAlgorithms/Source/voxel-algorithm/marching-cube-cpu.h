#pragma once

#include "voxel-generator.h"
#include "../mesh.h"
/*
class VoxelChunk;

class MarchingCubeCpu : public VoxelRenderer {
	public:
	MarchingCubeCpu(VoxelChunk* chunk, uint32_t drawShader);

	void initialize() override;

	void render(Camera* camera, unsigned int globalUBO) override;

	void destroy() override;

	~MarchingCubeCpu() {}

private:
	VoxelChunk* mChunk;
	uint32_t mDrawShader;

	std::vector<float> mDensityField;

	void generateMesh();
	void generateVertices(std::vector<Vertex>& vertices, std::vector<glm::uvec3>& splatArray);
	void generateIndices(std::vector<uint32_t>& indices, const std::vector<glm::uvec3>& splatArray);

	glm::vec4 createPoint(glm::ivec3 p);

	void update(Camera* camera);

	Mesh mMesh;

	bool mIntersected = false;
	glm::vec3 mIntersectionPoint = glm::vec3(0.0f);
};*/