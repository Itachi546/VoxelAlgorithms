#include "first-person-camera.h"

#include "glm-includes.h"
#include <glm/gtx/euler_angles.hpp>

FirstPersonCamera::FirstPersonCamera()
{
	mTargetPosition = mPosition = glm::vec3(0.0f, 1.0f, 3.0f);
	mTargetRotation = mRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	mFov = glm::radians(60.0f);
	mAspect = 4.0f / 3.0f;
	mNearPlane = 0.1f;
	mFarPlane = 1000.0f;

	mSpeed = 2.0f;
	mSensitivity = 0.3f;
}

void FirstPersonCamera::initialize() {
	calculateProjection();
	calculateView();
}

void FirstPersonCamera::update(float dt)
{
	//mPosition = mTargetPosition;
	//mRotation = mTargetRotation;

	mPosition.x += (mTargetPosition.x - mPosition.x) * 0.8f * dt;
	mPosition.y += (mTargetPosition.y - mPosition.y) * 0.8f * dt;
	mPosition.z += (mTargetPosition.z - mPosition.z) * 0.8f * dt;
	mRotation += (mTargetRotation - mRotation) * 0.8f * dt;

	calculateView();
	//DebugDraw::AddFrustum(mFrustumPoints, 8, 0xff0000);
}

glm::vec4 FirstPersonCamera::computeNDCCoordinate(const glm::vec3& p)
{
	// Calculate NDC Coordinate
	glm::vec4 ndc = mProjection * mView * glm::vec4(p, 1.0f);
	ndc /= ndc.w;

	// Convert in range 0-1
	ndc = ndc * 0.5f + 0.5f;
	// Invert Y-Axis
	ndc.y = 1.0f - ndc.y;
	return ndc;
}

glm::vec4 FirstPersonCamera::computeViewSpaceCoordinate(const glm::vec3& p)
{
	return mView * glm::vec4(p, 1.0f);
}

void FirstPersonCamera::calculateProjection()
{
	mProjection = glm::perspective(mFov, mAspect, mNearPlane, mFarPlane);
	mInvProjection  = glm::inverse(mProjection);
}

void FirstPersonCamera::calculateView()
{
	glm::mat3 rotation = glm::yawPitchRoll(mRotation.y, mRotation.x, mRotation.z);

	mForward = glm::normalize(rotation * glm::vec3(0.0f, 0.0f, 1.0f));
	mUp = glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f));

	mView = glm::lookAt(mPosition, mPosition + mForward, mUp);
	mInvView = glm::inverse(mView);

	mRight   = glm::vec3(mView[0][0], mView[1][0], mView[2][0]);
	mUp      = glm::vec3(mView[0][1], mView[1][1], mView[2][1]);
	mForward = glm::vec3(mView[0][2], mView[1][2], mView[2][2]);

	calculateFrustumPoints();

}

void FirstPersonCamera::calculateFrustumPoints()
{
	static const glm::vec3 frustumCorners[8] =
	{
		glm::vec3(-1.0f,  1.0f, -1.0f),  // NTL
		glm::vec3(1.0f,  1.0f, -1.0f),   // NTR
		glm::vec3(1.0f, -1.0f, -1.0f),   // NBR
		glm::vec3(-1.0f, -1.0f, -1.0f),  // NBL
		glm::vec3(-1.0f,  1.0f,  1.0f),  // FTL
		glm::vec3(1.0f,  1.0f,  1.0f),   // FTR
		glm::vec3(1.0f, -1.0f,  1.0f),   // FBR
		glm::vec3(-1.0f, -1.0f,  1.0f),  // FBL
	};

	// Project frustum corners into world space
	// inv(view) * inv(projection) * p
	// inv(A) * inv(B) = inv(BA)
	glm::mat4 invCam = glm::inverse(mFreezeVP);
	for (uint32_t i = 0; i < 8; i++)
	{
		glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
		mFrustumPoints[i] = invCorner / invCorner.w;
	}
}