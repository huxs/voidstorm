#pragma once

#define VOIDSTORM_MAX_POLYGON_VERTICES 1024
#define VOIDSTORM_EPSILON 0.000001

enum ShapeType
{
    POLYGON,
    CIRCLE,
    RAY,
    NONE
};

struct PolygonShape
{
    glm::vec2 computeCentroid();
    AABB computeAABB();

    void set(glm::vec2* vertices, uint16_t count);
    void setAsBox(float width, float height);
    void setAsBox(float width, float height, const glm::vec2& center);
    
    glm::vec2 vertices[VOIDSTORM_MAX_POLYGON_VERTICES];
    glm::vec2 normals[VOIDSTORM_MAX_POLYGON_VERTICES];
    glm::vec2 centroid;
    uint16_t count;
};

struct CircleShape
{
    float radius;
    float padd;
};

struct RayShape
{
    glm::vec2 direction;
};
