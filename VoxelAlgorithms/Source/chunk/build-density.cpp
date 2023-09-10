#include "build-density.h"

#include <glad/glad.h>

void DensityBuilder::generate(uint32_t texture, uint32_t width, uint32_t height, uint32_t depth)
{
	uint32_t sizeX = (width + 7 )  / 8;
	uint32_t sizeY = (height + 7 ) / 8;
	uint32_t sizeZ = (depth + 7 )  / 8;

	glUseProgram(mShader);

	glBindImageTexture(0, texture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(sizeX, sizeY, sizeZ);

	glUseProgram(0);
}
