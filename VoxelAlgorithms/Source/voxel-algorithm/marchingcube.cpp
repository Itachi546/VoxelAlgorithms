#include "marchingcube.h"
#include "marchingcube-lut.h"

#include "../glm-includes.h"
#include "../mesh.h"

#include <iostream>

void MarchingCube::initialize(uint32_t numVoxels)
{
	// Initialize Buffers
	mTriLUTBuffer = gl::createBuffer(MarchingCubeLUT::triTable, sizeof(MarchingCubeLUT::triTable), 0);

	const uint32_t splatBufferSize = numVoxels * numVoxels * numVoxels * 4 * sizeof(uint32_t);
	mSplatBuffer = gl::createBuffer(nullptr, splatBufferSize, GL_DYNAMIC_STORAGE_BIT);

	mCountBuffer = gl::createBuffer(nullptr, sizeof(uint32_t) * 8, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	mCountBufferPtr = static_cast<uint32_t*>(glMapNamedBuffer(mCountBuffer, GL_READ_WRITE));
	std::memset(mCountBufferPtr, 0, sizeof(uint32_t) * 8);
	assert(mCountBufferPtr != nullptr);

	// Initialize Shaders
	uint32_t listVerticesCS = gl::createShader("Assets/SPIRV/list-vertices.comp.spv");
	mListVerticesShader = gl::createProgram(&listVerticesCS, 1);
	gl::destroyShader(listVerticesCS);
	glProgramUniform1i(mListVerticesShader, 2, numVoxels);

	uint32_t listIndicesCS = gl::createShader("Assets/SPIRV/list-indices.comp.spv");
	mListIndicesShader = gl::createProgram(&listIndicesCS, 1);
	glProgramUniform1i(mListIndicesShader, 2, numVoxels);
	gl::destroyShader(listIndicesCS);

	// Initialize Timer Query
	gl::createQuery(mTimerQuery, 16);
}

void MarchingCube::generate(GpuMesh* mesh, uint32_t densityTexture, uint32_t numVoxel)
{
	generateVertices(mesh, densityTexture, numVoxel);

	// @TODO we can list nonempty grid in the first generateVertices pass and
	// trigger that number of invocation while generating indices, thus possibly 
	// reducing the total compute invocation and redundant calculations
	generateIndices(mesh, densityTexture, numVoxel);

	gl::lockBuffer(mSync);
	gl::waitBuffer(mSync);

	mesh->vertexCount = mCountBufferPtr[0];
	mesh->indexCount = mCountBufferPtr[4] * 3;
	std::memset(mCountBufferPtr, 0, sizeof(uint32_t) * 8);
}

void MarchingCube::generateVertices(GpuMesh* mesh, uint32_t densityTexture, uint32_t numVoxel)
{
	glBeginQuery(GL_TIME_ELAPSED, mTimerQuery[0]);

	glUseProgram(mListVerticesShader);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mTriLUTBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->vertexBuffer);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, mCountBuffer, 0, sizeof(uint32_t));
	glBindImageTexture(3, densityTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSplatBuffer);

	uint32_t workSize = (numVoxel + 7) / 8;
	glDispatchCompute(workSize, workSize, workSize);
	glUseProgram(0);

	glEndQuery(GL_TIME_ELAPSED);
}

void MarchingCube::generateIndices(GpuMesh* mesh, uint32_t densityTexture, uint32_t numVoxel)
{
	glBeginQuery(GL_TIME_ELAPSED, mTimerQuery[1]);

	// List indices
	glUseProgram(mListIndicesShader);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mTriLUTBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->indexBuffer);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, mCountBuffer, sizeof(uint32_t) * 4, sizeof(uint32_t));
	glBindImageTexture(3, densityTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSplatBuffer);

	uint32_t workSize = (numVoxel - 6) / 8;
	glDispatchCompute(workSize, workSize, workSize);
	glUseProgram(0);

	glEndQuery(GL_TIME_ELAPSED);

	// Wait until the query results are available
	int done = 0;
	while (!done) {
		glGetQueryObjectiv(mTimerQuery[1],
			GL_QUERY_RESULT_AVAILABLE,
			&done);
	}

	// get the query results
	uint64_t elapsedTimes[2] = { 0, 0 };
	glGetQueryObjectui64v(mTimerQuery[0], GL_QUERY_RESULT, &elapsedTimes[0]);
	glGetQueryObjectui64v(mTimerQuery[1], GL_QUERY_RESULT, &elapsedTimes[1]);

	timeToGenerateVertices = elapsedTimes[0] * 1e-6f;
	timeToGenerateIndices = elapsedTimes[1] * 1e-6f;

	std::cout << "\033[31;1;4mVertices Generation[" << timeToGenerateVertices << "ms ]"  <<
		          "Indices Generation[" << timeToGenerateIndices << "ms ]\033[0m" << std::endl;
}
/*
void MarchingCube::render(Camera* camera, unsigned int globalUBO)
{

	glm::mat4 M = glm::mat4(1.0f);

	glUseProgram(mDrawShader);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUBO);
	glUniformMatrix4fv(0, 1, GL_FALSE, &M[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glDrawElements(GL_TRIANGLES, mTriangleCount * 3, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
}
*/

void MarchingCube::destroy()
{
	gl::destroyProgram(mListVerticesShader);
	gl::destroyProgram(mListIndicesShader);

	gl::destroyBuffer(mTriLUTBuffer);
	gl::destroyBuffer(mCountBuffer);

	gl::destroyQuery(mTimerQuery, 16);

	mCountBufferPtr = nullptr;
}

