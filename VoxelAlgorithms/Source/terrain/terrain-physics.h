#pragma once

#include "../glm-includes.h"
#include <vector>

class DensityBuilder;

struct Rigidbody {
	glm::vec3 position;
	float mass = 1.0f;

	glm::vec3 prevPosition;
	float restitution = 0.1f;

	glm::vec3 velocity = glm::vec3(0.0f);
	float _pad1;
};

class TerrainPhysics {

public:
	TerrainPhysics() = default;

	void initialize(uint32_t densityParamUB);

	void addRigidbody(const Rigidbody& rb);

	void update(float dt);

	uint32_t mRBUniformBuffer;
	uint32_t mRBCount;
private:
	uint32_t mDensityParamUB;

	uint32_t mShader;

	const int kMaxRigidbodies = 1000;
};
