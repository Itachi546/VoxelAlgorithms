#pragma once

#include <stdint.h>

#include "../glm-includes.h"

struct DensityParams {
	float amplitude;
	float frequency;
	float lacunarity;
	float persistance;
	int octaves;
};

class DensityBuilder
{
public:
	DensityBuilder(DensityParams params, uint32_t shader);

	void generate(uint32_t texture, uint32_t width, uint32_t height, uint32_t depth, glm::ivec3 offset);

	float timeToGenerateDensity = 0.0f;
private:
	DensityParams mParams;
	unsigned int mShader;

	unsigned int mTimerQuery;
};