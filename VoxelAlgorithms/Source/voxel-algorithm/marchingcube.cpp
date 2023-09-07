#include "marchingcube.h"
#include "marchingcube-lut.h"
#include "../orbit-camera.h"

#define GLM_FORCE_XYZW_ONLY
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

MarchingCube::MarchingCube(VoxelChunk* chunk, unsigned int drawShader) : 
	mChunk(chunk),
	mTriLUTBuffer(~0u),
	mVertexBuffer(~0u),
	mCountBuffer(~0u),
	mCountBufferPtr(0),
	mListVerticesShader(~0u),
	mListIndicesShader(~0u),
	mSplatBuffer(~0u),
	mVertexCount(0),
	mDrawShader(drawShader)
{
}

void MarchingCube::initialize()
{
	mTriLUTBuffer = gl::createBuffer(MarchingCubeLUT::triTable, sizeof(MarchingCubeLUT::triTable), 0);

	uint32_t gridSize = mChunk->getNumVoxel();
	uint32_t gridCount = (gridSize - 1) * (gridSize - 1) * (gridSize - 1);
	const uint32_t vertexBufferSize = sizeof(float) * 6 * 3 * gridCount;
	mVertexBuffer = gl::createBuffer(nullptr, vertexBufferSize, GL_DYNAMIC_STORAGE_BIT);

	const uint32_t indexBufferSize = sizeof(uint32_t) * 15 * gridCount;
	mIndexBuffer = gl::createBuffer(nullptr, indexBufferSize, GL_DYNAMIC_STORAGE_BIT);

	const uint32_t splatBufferSize = gridSize * gridSize * gridSize * 4 * sizeof(uint32_t);
	mSplatBuffer = gl::createBuffer(nullptr, splatBufferSize, GL_DYNAMIC_STORAGE_BIT);

	mCountBuffer = gl::createBuffer(nullptr, sizeof(uint32_t) * 8, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	mCountBufferPtr = static_cast<uint32_t*>(glMapNamedBuffer(mCountBuffer, GL_READ_WRITE));
	std::memset(mCountBufferPtr, 0, sizeof(uint32_t) * 8);
	assert(mCountBufferPtr != nullptr);


	uint32_t listVerticesCS = gl::createShader("Assets/Shaders/list-vertices.comp");
	mListVerticesShader = gl::createProgram(&listVerticesCS, 1);
	gl::destroyShader(listVerticesCS);

	uint32_t listIndicesCS = gl::createShader("Assets/Shaders/list-indices.comp");
	mListIndicesShader = gl::createProgram(&listIndicesCS, 1);
	gl::destroyShader(listIndicesCS);

	generateMesh();
}

void MarchingCube::generateMesh()
{
	generateVertices();
	generateIndices();

	gl::lockBuffer(mSync);
	gl::waitBuffer(mSync);

	mVertexCount = mCountBufferPtr[0];
	mTriangleCount = mCountBufferPtr[4];
	std::memset(mCountBufferPtr, 0, sizeof(uint32_t) * 8);
	std::cout << "Vertices[ " << mVertexCount << " ]" << " Triangles[ " << mTriangleCount << " ]" << std::endl;
}

void MarchingCube::generateVertices()
{
	uint32_t gridSize = mChunk->getNumVoxel() - 1;
	glUseProgram(mListVerticesShader);
	glUniform1i(glGetUniformLocation(mListVerticesShader, "uVoxelCount"), gridSize);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mTriLUTBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mVertexBuffer);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, mCountBuffer, 0, sizeof(uint32_t));
	glBindImageTexture(3, mChunk->getDensityTexture(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSplatBuffer);

	glDispatchCompute((gridSize + 8) / 8, (gridSize + 8) / 8, (gridSize + 8) / 8);
	glUseProgram(0);
}

void MarchingCube::generateIndices()
{
	// @TODO maybe we can reduce shader invocations if we figure out the empty cell beforehand
	uint32_t gridSize = mChunk->getNumVoxel() - 1;

	// List indices
	glUseProgram(mListIndicesShader);
	glUniform1i(glGetUniformLocation(mListIndicesShader, "uVoxelCount"), gridSize);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mTriLUTBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mIndexBuffer);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, mCountBuffer, sizeof(uint32_t) * 4, sizeof(uint32_t));
	glBindImageTexture(3, mChunk->getDensityTexture(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSplatBuffer);

	glDispatchCompute((gridSize + 7) / 8, (gridSize + 7) / 8, (gridSize + 7) / 8);
	glUseProgram(0);
}

void MarchingCube::render(OrbitCamera* camera)
{

	glm::mat4 P = camera->getProjectionMatrix();
	glm::mat4 V = camera->getViewMatrix();

	glUseProgram(mDrawShader);
	glUniformMatrix4fv(glGetUniformLocation(mDrawShader, "P"), 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(mDrawShader, "V"), 1, GL_FALSE, &V[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glDrawElements(GL_TRIANGLES, mTriangleCount * 3, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
}

void MarchingCube::destroy()
{
	gl::destroyShader(mListVerticesShader);
	gl::destroyShader(mListIndicesShader);

	gl::destroyBuffer(mTriLUTBuffer);
	gl::destroyBuffer(mVertexBuffer);
	gl::destroyBuffer(mCountBuffer);

	mCountBufferPtr = nullptr;
}

