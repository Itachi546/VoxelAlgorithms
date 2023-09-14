#pragma once

#include <stdint.h>

#include "../glm-includes.h"

struct DensityParams {
	float amplitude;
	float frequency;
	float lacunarity;
	float persistence;
	int octaves;
};

class DensityBuilder
{
public:
	DensityBuilder(DensityParams params);

	void generate(uint32_t texture, uint32_t width, uint32_t height, uint32_t depth, glm::ivec3 offset);

	void destroy();

	float timeToGenerateDensity = 0.0f;

	unsigned int densityParamUB;
private:
	DensityParams mParams;
	unsigned int mShader;


	unsigned int mTimerQuery;
};