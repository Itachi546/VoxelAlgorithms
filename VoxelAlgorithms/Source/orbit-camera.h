#pragma once

#define GLM_FORCE_XYZW_ONLY

#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class OrbitCamera : public Camera {

public:
	OrbitCamera();

	void initialize() override;

	void update(float dt) override;

	glm::vec3 getPosition() const override { return mPosition; }

	void setPosition(const glm::vec3& position) { mPosition = position; }

	void setTarget(const glm::vec3& target) { mTarget = target; }

	void setZoom(float zoom) { mZoomLevel = zoom; }

	void changeRotation(float dtheta, float dphi) {
		mDeltaTheta = dtheta;
		mDeltaPhi = dphi;
	}

	void changeZoom(float dZoom) {
		mDeltaZoom = dZoom;
	}

	void setAspect(float aspect) { 
		mAspect = aspect; 
		updateProjectionMatrix();
	}

	void setNearPlane(float zNear) { 
		mNear = zNear;
		updateProjectionMatrix();
	}

	void setFarPlane(float zFar) { 
		mNear = zFar; 
		updateProjectionMatrix();
	}

	void setFov(float fovInRadians) { 
		mFovRadians = fovInRadians; 
		updateProjectionMatrix();
	}

	glm::mat4 getViewMatrix() const override { return mViewMatrix; }
	glm::mat4 getProjectionMatrix() const override { return mProjectionMatrix; }

private:
	glm::vec3 mPosition;

	float mPhi = 0.0f;
	float mDeltaPhi = 0.0f;

	float mTheta = 0.0f;
	float mDeltaTheta = 0.0f;
	
	glm::vec3 mTarget;

	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;

	float mAspect = 1.0f;
	float mNear = 0.3f;
	float mFar = 1000.0f;
	float mFovRadians = glm::radians(60.0f);

	float mRotateSpeed = 5.0f;

	float mZoomLevel = 20.0f;
	float mDeltaZoom = 0.0f;

	float mMinZoom = 5.0f;
	float mMaxZoom = FLT_MAX;
	float mZoomSpeed = 5.0f;

	float mDampingFactor = 0.93f;

	void createViewMatrix();
	void updateProjectionMatrix() {
		mProjectionMatrix = glm::perspective(mFovRadians, mAspect, mNear, mFar);
	}
};