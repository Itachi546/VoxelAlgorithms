#pragma once

#define GLM_FORCE_XYZW
#include "camera.h"

#include <memory>

class FirstPersonCamera : public Camera
{
public:
	FirstPersonCamera();

	void initialize() override;

	void update(float dt);
	glm::mat4 getProjectionMatrix() const override { return mProjection; }
	glm::mat4 getViewMatrix() const override { return mView; }

	glm::mat4 getInvProjectionMatrix() const { return mInvProjection; }
	glm::mat4 getInvViewMatrix()       const { return mInvView; }

	void setPosition(glm::vec3 p)
	{
		mPosition = p;
		mTargetPosition = p;
	}

	void setRotation(glm::vec3 rotation)
	{
		mRotation = mTargetRotation = rotation;
	}

	glm::vec3 getRotation() const 
	{
		return mRotation;
	}

	void setNearPlane(float d)
	{
		mNearPlane = d;
		calculateProjection();
	}

	void setFarPlane(float d)
	{
		mFarPlane = d;
		calculateProjection();
	}

	void setAspect(float aspect) 
	{
		mAspect = aspect;
		calculateProjection();
	}

	void setFOV(float fov)
	{
		mFov = fov;
		calculateProjection();
	}

	inline float getFOV() const { return mFov; }
	inline float getAspect() const { return mAspect; }
	inline float getNearPlane() const { return mNearPlane; }
	inline 	float getFarPlane()  const { return mFarPlane; }

	void walk(float dt)
	{
		mTargetPosition += mSpeed * dt * mForward;
	}

	void strafe(float dt)
	{
		mTargetPosition += mSpeed * dt * mRight;
	}

	void lift(float dt)
	{
		mTargetPosition += mSpeed * dt * mUp;
	}

	void rotate(float dx, float dy, float dt)
	{
		float m = mSensitivity * dt;
		mTargetRotation.y += dy * m;
		mTargetRotation.x += dx * m;

		constexpr float maxAngle = glm::radians(89.99f);
		mTargetRotation.x = glm::clamp(mTargetRotation.x, -maxAngle, maxAngle);
	}

	glm::vec4 computeNDCCoordinate(const glm::vec3& p);
	glm::vec4 computeViewSpaceCoordinate(const glm::vec3& p);

	glm::vec3 getForward() { return mForward; }
	glm::vec3 getPosition() const override { return mPosition; }
	
	glm::vec3 mFrustumPoints[8];
private:

	glm::vec3 mPosition;
	glm::vec3 mRotation;

	glm::vec3 mTargetPosition;
	glm::vec3 mTargetRotation;

	glm::mat4 mProjection, mInvProjection;
	float mFov;
	float mAspect;
	float mNearPlane;
	float mFarPlane;

	glm::mat4 mView, mInvView;
	glm::vec3 mRight;
	glm::vec3 mUp;
	glm::vec3 mForward;
	float mSpeed;
	float mSensitivity;

	bool mFreezeFrustum = false;
	glm::mat4 mFreezeVP;

	void calculateProjection();
	void calculateView();
	void calculateFrustumFast();
	void calculateFrustumPoints();

};