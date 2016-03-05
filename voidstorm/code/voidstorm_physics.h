#pragma once

struct ContactNode
{
    ContactNode() : reference(0), key(0), next(nullptr) {}

    uint32_t key;
    uint32_t reference;
    ContactNode* next;
};

struct ContactManager
{
    void add(dcutil::StackAllocator* stack, uint32_t key, uint32_t reference);
    uint32_t lookup(uint32_t key);
    
    ContactNode map[100];
};

struct PhysicsSimulator
{
    void integrateVelocity(World* world, float dt);
    void narrowCollision(World* world, float dt, LineRenderer* linerenderer);
    void resolveCollisions(World* world);
    void updateVelocity(World* world, float dt);
    
    Manifold manifolds[VOIDSTORM_PHYSICS_NUM_MANIFOLDS];
    uint32_t numManifolds;
};
