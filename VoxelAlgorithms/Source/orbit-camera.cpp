#include "orbit-camera.h"
#include "input.h"

OrbitCamera::OrbitCamera() :
	mPosition(glm::vec3(64.0f)),
	mTarget(glm::vec3(32.0f)),
	mViewMatrix(glm::mat4(1.0f)),
	mProjectionMatrix(glm::mat4(1.0f))
{

}

void OrbitCamera::initialize()
{
	mProjectionMatrix = glm::perspective(mFovRadians, mAspect, mNear, mFar);
}

void OrbitCamera::update(float dt)
{
	Input* input = Input::getInstance();

	mPhi += mDeltaPhi * mDampingFactor * mRotateSpeed * dt * 50.0f;

	mTheta += mDeltaTheta * mDampingFactor * mRotateSpeed * dt * 50.0f; 
	constexpr float thetaLimit = glm::radians(89.0f);
	mTheta = glm::clamp(mTheta, -thetaLimit, thetaLimit);

	mDeltaPhi *= mDampingFactor;
	mDeltaTheta *= mDampingFactor;

	mZoomLevel += mDeltaZoom * mDampingFactor * mZoomSpeed * dt * 50.0f;
	mZoomLevel = glm::clamp(mZoomLevel, mMinZoom, mMaxZoom);
	mDeltaZoom *= mDampingFactor;

	createViewMatrix();
}


void OrbitCamera::createViewMatrix()
{
	float cosTheta = cos(mTheta);
	mPosition = mTarget + mZoomLevel * glm::vec3(cosTheta * cos(mPhi),
		sin(mTheta),
		cosTheta * sin(mPhi));

	glm::vec3 forward = normalize(mTarget - mPosition);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	if (forward.y > 0.99)
		up = glm::vec3(1.0f, 0.0f, 0.0f);

	mViewMatrix = glm::lookAt(mPosition, mTarget, up);
}
