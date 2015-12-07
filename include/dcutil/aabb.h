#pragma once

#include "dcutil_helper.h"
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <cstdlib>

namespace dcutil
{
    struct DCUTIL_API AABB
    {
	AABB();
	AABB(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

	float getLengthX() const;
	float getLengthY() const;
	float getLengthZ() const;

	glm::vec3 getCenter() const;
        bool testPoint(const glm::vec3& point) const;

	void expand(const AABB& aabb);
	void expand(const glm::vec3& point);
       
	union
	{
	    struct
	    {
		float m_minX, m_minY, m_minZ, m_maxX, m_maxY, m_maxZ;
	    };
	    
	    float arr[6];
	};
    };

    inline AABB::AABB()
	    : m_minX(FLT_MAX), m_maxX(-FLT_MAX), m_minY(FLT_MAX), m_maxY(-FLT_MAX), m_minZ(FLT_MAX), m_maxZ(-FLT_MAX)
    {}

    inline AABB::AABB(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
	    : m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY), m_minZ(minZ), m_maxZ(maxZ)
    {}
    
    inline float AABB::getLengthX() const
    {
	return glm::abs(m_maxX - m_minX); 
    }

    inline float AABB::getLengthY() const
    {
	return glm::abs(m_maxY - m_minY); 
    }

    inline float AABB::getLengthZ() const
    {
	return glm::abs(m_maxZ - m_minZ); 
    }

    inline glm::vec3 AABB::getCenter() const
    {
	return glm::vec3(m_minX + getLengthX() / 2, m_minY + getLengthY() / 2, m_minZ + getLengthZ() / 2);
    }

    inline bool AABB::testPoint(const glm::vec3& point) const
    {
	if(point.x > m_minX && point.x < m_maxX)
	{
	    if(point.y > m_minY && point.y < m_maxY)
	    {
		if(point.z > m_minZ && point.z < m_maxZ)
		{
		    return true;
		}
	    }
	}
	
	return false;
    }
}
