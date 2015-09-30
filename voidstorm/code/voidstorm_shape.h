#pragma once

#define VOIDSTORM_MAX_POLYGON_VERTICES 1024

enum ShapeType
{
    POLYGON,
    CIRCLE,
    NONE
};

struct ContactResult
{
    ContactResult()
	    : hit(false){}
    
    bool hit;
    glm::vec2 normal;
    glm::vec2 position;
    
    float r;
};

typedef ContactResult CONTACT_CALLBACK(World* world,
				       glm::vec2 deltaVel,
				       TransformManager::Instance transformA,
				       CollisionManager::ShapeData shapeA,
				       TransformManager::Instance transformB,
				       CollisionManager::ShapeData shapeB);

extern CONTACT_CALLBACK* g_contactCallbacks[ShapeType::NONE][ShapeType::NONE];

void setupContactCallbacks();

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
};

