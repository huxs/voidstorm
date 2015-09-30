#pragma once

#include "dcutil_helper.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace dcutil
{
    class Orientation
    {
    public:
	DCUTIL_API void setOrientation(glm::mat3& rotationMatrix);
	DCUTIL_API void setOrientation(float x, float y, float z);
	DCUTIL_API void setOrientation(float angle, glm::vec3& axis);
	DCUTIL_API void setOrientation(glm::quat quat);
	
	DCUTIL_API glm::vec3 getRight() const;
	DCUTIL_API glm::vec3 getFront() const;
	DCUTIL_API glm::vec3 getUp() const;

	DCUTIL_API float getAngle() const;
	DCUTIL_API glm::vec3 getAxis() const;

	DCUTIL_API void yaw(float angle);
	DCUTIL_API void pitch(float angle);
	DCUTIL_API void roll(float angle);

	DCUTIL_API void yawGlobal(float angle);
	DCUTIL_API void pitchGlobal(float angle);
	DCUTIL_API void rollGlobal(float angle);

    private:
	void rotate(glm::quat quat);
	void rotateGlobal(glm::quat quat);

	glm::quat m_quat;
    };
}
