#pragma once

#include <glm/gtx/transform.hpp>

/* AABB */
// NOTE (daniel): Mabey rename this to Rectangle?
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

/* Transform */

struct Rotation
{
    Rotation()
	    :
	    s(0.0f),
	    c(0.0f)
	{}
    Rotation(float rotation)
	    :
	    s(sin(rotation)),
	    c(cos(rotation))
	{}
    
    float s;
    float c;
};

struct Transform
{
    Transform() {}
    Transform(glm::vec2 position, float rotation)
	    :
	    p(position),
	    q(rotation)
	{}
   
    glm::vec2 rotate(const glm::vec2& v) const;
    glm::vec2 rotateInverse(const glm::vec2& v) const;
    
    glm::vec2 translate(const glm::vec2& v) const;
    glm::vec2 translateInverse(const glm::vec2& v) const;
    
    glm::vec2 mul(const glm::vec2& v) const;
    glm::vec2 mulInverse(const glm::vec2& v) const;
    
    glm::vec2 p;
    Rotation q;
};

// Multiply two rotations
inline Rotation mulRot(const Rotation& a, const Rotation& b)
{
    Rotation result;

    result.s = a.c * b.s + a.s * b.c;
    result.c = a.c * b.c - a.s * b.s;
    
    return result;
}

// Transpose multiply two rotations
inline Rotation mulRotT(const Rotation& a, const Rotation& b)
{
    Rotation result;

    result.s = a.c * b.s - a.s * b.c;
    result.c = a.c * b.c + a.s * b.s;
    
    return result;
}

inline glm::vec2 Transform::rotate(const glm::vec2& v) const
{
    glm::vec2 result;

    result.x = v.x * q.c + v.y * q.s;
    result.y = v.x * -q.s + v.y * q.c;

    return result;
}

inline glm::vec2 Transform::rotateInverse(const glm::vec2& v) const
{
    glm::vec2 result;

    result.x = v.x * q.c - v.y * q.s;
    result.y = v.x * q.s + v.y * q.c;

    return result;
}

inline glm::vec2 Transform::translate(const glm::vec2& v) const
{
    glm::vec2 result;

    result = v + p;

    return result;
}

inline glm::vec2 Transform::translateInverse(const glm::vec2& v) const
{
    glm::vec2 result;

    result = v - p;

    return result;
}

inline glm::vec2 Transform::mul(const glm::vec2& v) const
{
    glm::vec2 result;
    
    result = rotate(v);
    result = translate(result);
    
    return result;
}

inline glm::vec2 Transform::mulInverse(const glm::vec2& v) const
{
    glm::vec2 result;

    result = translateInverse(v);
    result = rotateInverse(result);   
    
    return result;
}

inline Transform mulTrans(const Transform& a, const Transform& b)
{
    Transform result;

    result.q = mulRot(a.q, b.q);
    result.p = a.rotate(b.p) + a.p;   
    
    return result;
}

inline Transform mulTransT(const Transform& a, const Transform& b)
{
    Transform result;

    result.q = mulRotT(a.q, b.q);
    result.p = a.rotateInverse(b.p - a.p);
    
    return result;
}
     



