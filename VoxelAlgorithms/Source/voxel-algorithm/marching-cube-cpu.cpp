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
	modelMatrixBuffer = gl::createBuffer(nullptr, sizeof(glm::mat4) * 5, GL_DYNAMIC_STORAGE_BIT);
}

static float getDensity(glm::vec3 p)
{
	float d = std::min(p.y - 8.0f, glm::length(p - 16.0f) - 8.0f);

	float d1 = length(glm::vec2(p.x, p.y) - 17.0f) - 6.0f;
	//d = std::max(d, -d1);
	return d;
}

void MarchingCubeCpu::initialize()
{
	uint32_t gridSize = mChunk->getNumVoxel();
	mDensityField.resize(gridSize * gridSize * gridSize);
	for (uint32_t y = 0; y < gridSize; ++y) {
		for (uint32_t z = 0; z < gridSize; ++z) {
			for (uint32_t x = 0; x < gridSize; ++x) {
				glm::uvec3 uv = glm::vec3(x, y, z);
				int	index = uv.z * gridSize * gridSize + uv.y * gridSize + uv.x;
				mDensityField[index] = getDensity(uv);
			}
		}
	}
}


void MarchingCubeCpu::render(Camera* camera, unsigned int globalUBO)
{
	update(camera);
	generateMesh();

	glUseProgram(mDrawShader);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUBO);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, modelMatrixBuffer, 0, sizeof(glm::mat4));

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), mIntersectionPoint);
	glNamedBufferSubData(modelMatrixBuffer, 0, sizeof(glm::mat4), &transform);
	if (mIntersected) {
		glBindBufferRange(GL_UNIFORM_BUFFER, 1, modelMatrixBuffer, 0, sizeof(glm::mat4));
		Mesh* sphere = DefaultMesh::getInstance()->getSphere();
		sphere->draw();
	}

	transform = glm::mat4(1.0f);
	glNamedBufferSubData(modelMatrixBuffer, 0, sizeof(glm::mat4), &transform);

	mMesh.draw();

	glUseProgram(0);

	mIntersected = false;
}

void MarchingCubeCpu::destroy()
{
	mMesh.destroy();
}

void MarchingCubeCpu::generateMesh()
{
	mMesh.destroy();
	uint32_t gridSize = mChunk->getNumVoxel();
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<glm::uvec3> splatArray(gridSize * gridSize * gridSize);

	generateVertices(vertices, splatArray);
	generateIndices(indices, splatArray);
	mMesh.create(vertices, indices);
}

glm::vec3 interpolatePosition(glm::vec4 p0, glm::vec4 p1, float isoLevel) {
	float d0 = p0.w;
	float d1 = p1.w;

	float d = (isoLevel - d0) / (d1 - d0);
	return glm::vec3(p0) + d * glm::vec3(p1 - p0);
}

static bool intersectBox(glm::vec3 r0, glm::vec3 rd, glm::vec3 boxSize, glm::vec3& p0, glm::vec3& p1) {

	glm::vec3 invDir = 1.0f / rd;

	glm::vec3 tmin = -r0 * invDir;
	glm::vec3 tmax = boxSize * glm::abs(invDir) - r0 * invDir;

	float t0 = std::max(tmin.x, std::max(tmin.y, tmin.z));
	float t1 = std::min(tmax.x, std::min(tmax.y, tmax.z));

	if (t0 > t1 || t1 < 0.0f) return false;

	if (t0 >= 0.0f)
		p0 = r0 + t0 * rd;
	else p0 = r0;

	p1 = r0 + t1 * rd;

	return true;
}

void MarchingCubeCpu::update(Camera* camera)
{
	glm::vec2 mousePosNormalized = Input::getInstance()->getMousePos() / glm::vec2(mWidth, mHeight);
	glm::vec4 ndcCoord = glm::vec4(mousePosNormalized.x * 2.0f - 1.0f, 1.0f - 2.0f * mousePosNormalized.y, -1.0f, 1.0f);

	glm::mat4 invP = glm::inverse(camera->getProjectionMatrix());
	glm::mat4 invV = glm::inverse(camera->getViewMatrix());
	glm::vec4 viewPos = invP * ndcCoord;
	glm::vec4 worldPos = invV * glm::vec4(viewPos.x, viewPos.y, -1.0f, 0.0f);

	glm::vec3 rd = glm::normalize(glm::vec3(worldPos));
	glm::vec3 r0 = camera->getPosition();

	int gridSize = mChunk->getNumVoxel();
	

	/*
	float d = 0.1f;
	for (int i = 0; i < 256; ++i) {
		glm::vec3 p = r0 + d * rd;
		float t = getDensity(p);
		if (t < 0.001) {
			mIntersected = true;
			mIntersectionPoint = p;
			break;
		}
		d += t;
	}

	int radius = 2;
	int gridSize = mChunk->getNumVoxel();
	if (mIntersected) {
		for (int z = -radius; z <= radius; z++) {
			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					glm::ivec3 p = glm::ivec3(glm::floor(mIntersectionPoint + glm::vec3(x, y, z)));
					if (p.x < 0 || p.x > gridSize - 1 || p.y < 0 || p.y > gridSize - 1 || p.z < 0 || p.z > gridSize - 1) continue;
					int	index = p.z * gridSize * gridSize + p.y * gridSize + p.x;
					float brush = glm::length(mIntersectionPoint - glm::vec3(p)) - float(radius);
					mDensityField[index] = std::max(mDensityField[index], -brush);
				}
			}
		}
	}
	*/
	//glm::vec3 p0, p1;
	//if (!intersectBox(r0, rd, glm::vec3(gridSize - 1), p0, p1)) return;
	//r0 = p0;

	// Fast Ray Voxel Traversal
	glm::ivec3 ip = glm::floor(r0);
	glm::ivec3 stepSize = glm::sign(rd);
	glm::vec3 tMax = (glm::vec3(ip + stepSize) - r0) / rd;
	glm::vec3 tDelta = glm::vec3(stepSize) / rd;

	float maxDist = 100.0;// glm::length(p1 - p0);
	float t = glm::max(tMax.x, glm::max(tMax.y, tMax.z));

	float lastDensity = 0.0f;
	glm::ivec3 lastPosition = ip;

	while (t < maxDist) {

		if (tMax.x < tMax.y) {
			if (tMax.x < tMax.z) {
				ip.x += stepSize.x;
				tMax.x += tDelta.x;
				t = tMax.x;
			}
			else {
				ip.z += stepSize.z;
				tMax.z += tDelta.z;
				t = tMax.z;
			}
		}
		else {
			if (tMax.y < tMax.z) {
				ip.y += stepSize.y;
				tMax.y += tDelta.y;
				t = tMax.y;
			}
			else {
				ip.z += stepSize.z;
				tMax.z += tDelta.z;
				t = tMax.z;
			}
		}
		if (ip.x < 0 || ip.x > gridSize - 1 || ip.y < 0 || ip.y > gridSize - 1 || ip.z < 0 || ip.z > gridSize - 1) break;

		int	index = ip.z * gridSize * gridSize + ip.y * gridSize + ip.x;
		float density = mDensityField[index];

		if (density <= 0.0001f) {
			mIntersected = true;
			mIntersectionPoint = interpolatePosition(glm::vec4(lastPosition, lastDensity), glm::vec4(ip, density), 0.0001f);
			break;
		}

		lastPosition = ip;
		lastDensity = density;
	}

	int radius = 2;
	if (mIntersected && Input::getInstance()->wasKeyPressed(1)) {
		for (int z = -radius; z <= radius; z++) {
			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					glm::ivec3 p = glm::ivec3(glm::floor(mIntersectionPoint + glm::vec3(x, y, z)));
					if (p.x < 0 || p.x > gridSize - 1 || p.y < 0 || p.y > gridSize - 1 || p.z < 0 || p.z > gridSize - 1) continue;
					int	index = p.z * gridSize * gridSize + p.y * gridSize + p.x;
					float brush = glm::length(mIntersectionPoint - glm::vec3(p)) - float(radius);
					mDensityField[index] = std::max(mDensityField[index], -brush);
				}
			}
		}
	}

}

glm::vec4 MarchingCubeCpu::createPoint(glm::ivec3 p) {
	const int gridSize = mChunk->getNumVoxel();
	int	index = p.z * gridSize * gridSize + p.y * gridSize + p.x;
	return glm::vec4(p.x, p.y, p.z, mDensityField[index]);
}

static glm::vec3 calculateNormal(glm::ivec3 p, const std::vector<float>& densityField, int gridSize) {

	auto density = [&](const glm::ivec3& ip) {
		int index = ip.z * gridSize * gridSize + ip.y * gridSize + ip.x;
		return densityField[index];
		};

	float d = density(p);
	return glm::normalize(glm::vec3(
		density(p + glm::ivec3(1, 0, 0)) - d,
		density(p + glm::ivec3(0, 1, 0)) - d,
		density(p + glm::ivec3(0, 0, 0)) - d
	));
}

glm::vec3 interpolatePosition(glm::vec4 p0, glm::vec4 p1, float isoLevel, glm::vec3& n, const std::vector<float>& densityField, int gridSize) {
	float d0 = p0.w;
	float d1 = p1.w;

	float d = (isoLevel - d0) / (d1 - d0);

	glm::vec3 n0 = calculateNormal(p0, densityField, gridSize);
	glm::vec3 n1 = calculateNormal(p1, densityField, gridSize);
	n = glm::normalize(n0 + d * (n1 - n0));
	return glm::vec3(p0) + d * glm::vec3(p1 - p0);
}

void MarchingCubeCpu::generateVertices(std::vector<Vertex>& vertices, std::vector<glm::uvec3>& splatArray)
{
	uint32_t gridSize = mChunk->getNumVoxel() - 1;
	uint32_t totalVertices = 0;

	for (uint32_t y = 0; y < gridSize; ++y) {
		for (uint32_t z = 0; z < gridSize; ++z) {
			for (uint32_t x = 0; x < gridSize; ++x) {
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
					glm::vec3 p = interpolatePosition(cubeCorners[CornerAFromEdge[e]], cubeCorners[CornerBFromEdge[e]], isoLevel, n, mDensityField, gridSize);

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

	for (uint32_t y = 0; y < gridSize - 1; ++y) {
		for (uint32_t z = 0; z < gridSize - 1; ++z) {
			for (uint32_t x = 0; x < gridSize - 1; ++x) {

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
