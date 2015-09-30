#pragma once

#include "dcutil_helper.h"
#include <glm/glm.hpp>

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
        bool testPoint(glm::vec3 point) const;

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

/*
//TODO: Should we use a rectangle struct?
inline Rectangle AABB::getXZRect() const
{
return Rectangle(m_minX, m_minZ, getLengthX(), getLengthZ());
}
*/

    inline glm::vec3 AABB::getCenter() const
    {
	return glm::vec3(m_minX + getLengthX() / 2, m_minY + getLengthY() / 2, m_minZ + getLengthZ() / 2);
    }

    inline bool AABB::testPoint(glm::vec3 point) const
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
