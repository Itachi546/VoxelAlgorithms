#pragma once

#include <stdint.h>
#include <vector>
#include <iostream>

constexpr uint32_t INVALID_RESOURCE_HANDLE = ~0u;

template <typename Resource>
struct ResourcePool {

	void initialize(uint32_t poolSize) {
		this->poolSize = poolSize;
		resourceMemory.resize(poolSize);

		for (uint32_t i = 0; i < poolSize; ++i)
			freeIndices.push_back(poolSize - i - 1);

		usedIndicesCount = 0;
	}

	uint32_t obtainResource() {
		if (freeIndices.size() > 0) {
			uint32_t index = freeIndices.back();
			freeIndices.pop_back();
			usedIndicesCount++;
			return index;
		}
		std::cerr << "Resource Space Unavailable" << std::endl;
		return INVALID_RESOURCE_HANDLE;
	}

	Resource* accessResource(uint32_t handle) {
		if (handle != INVALID_RESOURCE_HANDLE) {
			return &resourceMemory[handle];
		}
		return nullptr;
	}

	void releaseResource(uint32_t handle) {
		auto found = std::find(freeIndices.begin(), freeIndices.end(), handle);
		if (found == freeIndices.end()) {
			freeIndices.push_back(handle);
			usedIndicesCount--;
		}
		else 
			std::cerr << "Releasing already destroyed resources" << std::endl;
	}

	void destroy() {
		if (usedIndicesCount) {
			resourceMemory.clear();
			usedIndicesCount = 0;
		}
		freeIndices.clear();
	}


	uint32_t poolSize = 16;
	std::vector<uint32_t> freeIndices;
	std::vector<Resource> resourceMemory;
	uint32_t usedIndicesCount;
};