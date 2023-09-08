#include "marching-cube-cpu.h"
#include "marchingcube-lut.h"

#include "../voxel-chunk.h"
#include "../orbit-camera.h"
#include "../input.h"


static const int CornerAFromEdge[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
static const int CornerBFromEdge[12] = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };

// Given a edge 0..11, it returns the parent cell where the vertex is generated 
const glm::ivec4 EdgeToCellVertex[12] = {
	glm::ivec4(0, 0, 0, 0), // 0,
	glm::ivec4(1, 0, 0, 1), // X
	glm::ivec4(0, 0, 1, 0), // Z
	glm::ivec4(0, 0, 0, 1), // 0
	glm::ivec4(0, 1, 0, 0),  // Y
	glm::ivec4(1, 1, 0, 1),  // XY
	glm::ivec4(0, 1, 1, 0), // YZ
	glm::ivec4(0, 1, 0, 1), // Y
	glm::ivec4(0, 0, 0, 2), // 0
	glm::ivec4(1, 0, 0, 2), // X
	glm::ivec4(1, 0, 1, 2), // XZ
	glm::ivec4(0, 0, 1, 2)  // Z
};

MarchingCubeCpu::MarchingCubeCpu(VoxelChunk* chunk, uint32_t drawShader) : mChunk(chunk), mDrawShader(drawShader)
{
}

void MarchingCubeCpu::initialize()
{
	generateMesh();
}

void MarchingCubeCpu::render(Camera* camera, unsigned int globalUBO)
{

	glUseProgram(mDrawShader);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUBO);

	mMesh.draw();

	glUseProgram(0);
}

void MarchingCubeCpu::destroy()
{
	mMesh.destroy();
}

void MarchingCubeCpu::generateMesh()
{
	uint32_t gridSize = mChunk->getNumVoxel();
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<glm::uvec3> splatArray(gridSize * gridSize * gridSize);

	generateVertices(vertices, splatArray);
	generateIndices(indices, splatArray);
	mMesh.create(vertices, indices);
}

static float getDensity(glm::vec3 p)
{
	return std::min(p.y - 8.0f, glm::length(p - 17.0f) - 8.0f);
}

static glm::vec4 createPoint(glm::vec3 p) {
	return glm::vec4(p.x, p.y, p.z, getDensity(p));
}

static glm::vec3 calculateNormal(glm::vec3 p) {
	return glm::normalize(glm::vec3(
		getDensity(p + glm::vec3(1.0f, 0.0f, 0.0f)) - getDensity(p - glm::vec3(1.0f, 0.0f, 0.0f)),
		getDensity(p + glm::vec3(0.0f, 1.0f, 0.0f)) - getDensity(p - glm::vec3(0.0f, 1.0f, 0.0f)),
		getDensity(p + glm::vec3(0.0f, 0.0f, 0.0f)) - getDensity(p - glm::vec3(0.0f, 0.0f, 1.0f))
	));
}

glm::vec3 interpolatePosition(glm::vec4 p0, glm::vec4 p1, float isoLevel, glm::vec3& n) {
	float d0 = p0.w;
	float d1 = p1.w;

	float d = (isoLevel - d0) / (d1 - d0);

	glm::vec3 n0 = calculateNormal(p0);
	glm::vec3 n1 = calculateNormal(p1);
	n = glm::normalize(n0 + d * (n1 - n0));
	return glm::vec3(p0) + d * glm::vec3(p1 - p0);
}

void MarchingCubeCpu::generateVertices(std::vector<Vertex>& vertices, std::vector<glm::uvec3>& splatArray)
{
	uint32_t gridSize = mChunk->getNumVoxel() - 1;
	uint32_t totalVertices = 0;

	for (uint32_t y = 0; y <= gridSize; ++y) {
		for (uint32_t z = 0; z <= gridSize; ++z) {
			for (uint32_t x = 0; x <= gridSize; ++x) {
				glm::uvec3 uv = glm::vec3(x, y, z);
				// 8 corners of the current cube
				glm::vec4 cubeCorners[8] = {
					createPoint(glm::vec3(uv.x, uv.y, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y, uv.z + 1)),
					createPoint(glm::vec3(uv.x, uv.y, uv.z + 1)),
					createPoint(glm::vec3(uv.x, uv.y + 1, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y + 1, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y + 1, uv.z + 1)),
					createPoint(glm::vec3(uv.x, uv.y + 1, uv.z + 1))
				};

				const float isoLevel = 0.0f;

				int edgeIndex = 0;
				if (cubeCorners[0].w < isoLevel) edgeIndex |= 1;
				if (cubeCorners[1].w < isoLevel) edgeIndex |= 2;    // 1 << 1;
				if (cubeCorners[2].w < isoLevel) edgeIndex |= 4;    // 1 << 2;
				if (cubeCorners[3].w < isoLevel) edgeIndex |= 8;    // 1 << 3;
				if (cubeCorners[4].w < isoLevel) edgeIndex |= 16;   // 1 << 4;
				if (cubeCorners[5].w < isoLevel) edgeIndex |= 32;   // 1 << 5;
				if (cubeCorners[6].w < isoLevel) edgeIndex |= 64;   // 1 << 6;
				if (cubeCorners[7].w < isoLevel) edgeIndex |= 128;  // 1 << 7;

				if (edgeIndex == 0) continue;

				int* triangulation = MarchingCubeLUT::triTable[edgeIndex];
				glm::uvec3 values = glm::uvec3(~0u);

				for (int i = 0; i < 15; ++i) {
					int e = triangulation[i];

					// To reuse the vertex we only generate vertex if it lies on edge 0, 3, or 8
					if (e == -1 || (e != 0 && e != 3 && e != 8) || values[e / 3] != ~0u) continue;

					glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
					glm::vec3 p = interpolatePosition(cubeCorners[CornerAFromEdge[e]], cubeCorners[CornerBFromEdge[e]], isoLevel, n);

					vertices.emplace_back(Vertex{ p, n });

					int offset = e / 3;
					values[offset] = totalVertices;
					totalVertices += 1;
				}
				int	splatIndex = uv.z * (gridSize + 1) * (gridSize + 1) + uv.y * (gridSize + 1) + uv.x;
				splatArray[splatIndex] = glm::uvec4(values, ~0u);
			}
		}
	}
}

static uint32_t getVertexIndex(glm::ivec3 uv, int edge, uint32_t gridSize, const std::vector<glm::uvec3>& splatArray) {
	glm::ivec4 offset = EdgeToCellVertex[edge];
	uv += glm::vec3(offset);

	int	splatIndex = uv.z * (gridSize + 1) * (gridSize + 1) + uv.y * (gridSize + 1) + uv.x;
	return splatArray[splatIndex][offset.w];
}

void MarchingCubeCpu::generateIndices(std::vector<uint32_t>& indices, const std::vector<glm::uvec3>& splatArray)
{
	uint32_t gridSize = mChunk->getNumVoxel() - 1;
	uint32_t totalTriangle = 0;

	for (uint32_t y = 0; y < gridSize; ++y) {
		for (uint32_t z = 0; z < gridSize; ++z) {
			for (uint32_t x = 0; x < gridSize; ++x) {

				glm::ivec3 uv = glm::ivec3(x, y, z);
				// 8 corners of the current cube
				glm::vec4 cubeCorners[8] = {
					createPoint(glm::vec3(uv.x, uv.y, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y, uv.z + 1)),
					createPoint(glm::vec3(uv.x, uv.y, uv.z + 1)),
					createPoint(glm::vec3(uv.x, uv.y + 1, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y + 1, uv.z)),
					createPoint(glm::vec3(uv.x + 1, uv.y + 1, uv.z + 1)),
					createPoint(glm::vec3(uv.x, uv.y + 1, uv.z + 1))
				};

				const float isoLevel = 0.0f;

				int edgeIndex = 0;
				if (cubeCorners[0].w < isoLevel) edgeIndex |= 1;
				if (cubeCorners[1].w < isoLevel) edgeIndex |= 2;    // 1 << 1;
				if (cubeCorners[2].w < isoLevel) edgeIndex |= 4;    // 1 << 2;
				if (cubeCorners[3].w < isoLevel) edgeIndex |= 8;    // 1 << 3;
				if (cubeCorners[4].w < isoLevel) edgeIndex |= 16;   // 1 << 4;
				if (cubeCorners[5].w < isoLevel) edgeIndex |= 32;   // 1 << 5;
				if (cubeCorners[6].w < isoLevel) edgeIndex |= 64;   // 1 << 6;
				if (cubeCorners[7].w < isoLevel) edgeIndex |= 128;  // 1 << 7;

				if (edgeIndex == 0) continue;

				int* triangulation = MarchingCubeLUT::triTable[edgeIndex];
				for (int i = 0; i < 15; i += 3) {
					int e0 = triangulation[i];
					if (e0 == -1) break;

					int e1 = triangulation[i + 1];
					int e2 = triangulation[i + 2];

					uint32_t i0 = getVertexIndex(uv, e0, gridSize, splatArray);
					uint32_t i1 = getVertexIndex(uv, e1, gridSize, splatArray);
					uint32_t i2 = getVertexIndex(uv, e2, gridSize, splatArray);

					indices.push_back(i0);
					indices.push_back(i1);
					indices.push_back(i2);

					totalTriangle += 1;
				}

			}
		}
	}
}
