#pragma once

struct AABB
{
    glm::vec2 getCenter();
    float getPerimiter();

    void combine(const AABB& other);
    void combine(const AABB& first, const AABB& second);
     
    glm::vec2 lower;
    glm::vec2 upper;
};

inline glm::vec2 AABB::getCenter()
{
    return 0.5f * (lower + upper);
}

inline float AABB::getPerimiter()
{
    float w = upper.x - lower.x;
    float h = upper.y - lower.y;
    return 2.0f * (w + h);
}

inline void AABB::combine(const AABB& other)
{
    lower = glm::min(lower, other.lower);
    upper = glm::max(upper, other.upper);
}

inline void AABB::combine(const AABB& first, const AABB& second)
{
    lower = glm::min(first.lower, second.lower);
    upper = glm::max(first.upper, second.upper);
}

inline bool aabbOverlap(const AABB& a, const AABB& b)
{
    glm::vec2 d1, d2;
    d1 = b.lower - a.upper;
    d2 = a.lower - b.upper;

    if (d1.x > 0.0f || d1.y > 0.0f)
	return false;

    if (d2.x > 0.0f || d2.y > 0.0f)
	return false;

    return true;
}
