#include "terrain-physics.h"
#include "build-density.h"

#include "../gl.h"

void TerrainPhysics::initialize(uint32_t densityParamUB)
{
	mDensityParamUB = densityParamUB;

	mRBUniformBuffer = gl::createBuffer(nullptr, sizeof(Rigidbody) * kMaxRigidbodies, GL_DYNAMIC_STORAGE_BIT);

	uint32_t compShader = gl::createShader("Assets/SPIRV/terrain-physics.comp.spv");
	mShader = gl::createProgram(&compShader, 1);
	gl::destroyShader(compShader);

}

void TerrainPhysics::addRigidbody(const Rigidbody& rb)
{
	glNamedBufferSubData(mRBUniformBuffer, mRBCount * sizeof(Rigidbody), sizeof(Rigidbody), &rb);
	mRBCount++;
}

void TerrainPhysics::update(float dt)
{
	uint32_t localSizeX = (mRBCount + 7) / 8;
	if (localSizeX == 0) return;

	glUseProgram(mShader);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mRBUniformBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 8, mDensityParamUB);
	glUniform1i(0, mRBCount);
	glUniform1f(1, dt);
	glDispatchCompute(localSizeX, 1, 1);
	glUseProgram(0);
}
