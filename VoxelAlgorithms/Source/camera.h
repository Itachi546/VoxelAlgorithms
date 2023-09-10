#pragma once

#include "glm-includes.h"

class Camera {

public:
	virtual void initialize() = 0;
	virtual void update(float dt) = 0;
	
	virtual  glm::mat4 getProjectionMatrix() const = 0;
	virtual  glm::mat4 getViewMatrix() const = 0;

	virtual glm::vec3 getPosition() const = 0;

	virtual ~Camera(){}
};