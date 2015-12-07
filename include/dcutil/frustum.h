#pragma once

#include "dcutil_helper.h"
#include "aabb.h"

namespace dcutil
{
    struct Frustum
    {
	DCUTIL_API void calculatePlanes(const glm::mat4& view, const glm::mat4& proj);
        
	DCUTIL_API bool testAABB(const dcutil::AABB& aabb);
	DCUTIL_API bool testSphere(const glm::vec3& center, float radius, bool ignoreNearZ);

	glm::vec4 m_planes[6];
    };

}
