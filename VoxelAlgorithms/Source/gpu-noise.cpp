#include "gpu-noise.h"
#include "gl.h"
#include <random>

void GpuNoiseGenerator::initialize() {
	unsigned int cs = gl::createShader("../Assets/Shaders/noise.comp");
	mNoiseShader = gl::createProgram(&cs, 1);
	gl::destroyShader(cs);

	auto rng = std::default_random_engine{};
	std::shuffle(std::begin(mPermutationArray), std::end(mPermutationArray), rng);
	mPermutationBuffer = gl::createBuffer(mPermutationArray.data(), (uint32_t)(mPermutationArray.size() * sizeof(int)), 0);
}

void GpuNoiseGenerator::generateNoise(unsigned int texture, uint32_t textureId, uint32_t width, uint32_t height, uint32_t layerCount)
{
	glUseProgram(mNoiseShader);
	uint32_t workGroupX = (width + 31) / 32;
	uint32_t workGroupY = (height + 31) / 32;

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mPermutationBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, texture, 0, GL_TRUE, layerCount, GL_WRITE_ONLY, GL_RED);

	glDispatchCompute(workGroupX, workGroupY, layerCount);
	glUseProgram(0);
}

void GpuNoiseGenerator::destroy() {
	gl::destroyProgram(mNoiseShader);
}
