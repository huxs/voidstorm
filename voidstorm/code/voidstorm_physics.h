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
    void simulate(World* world, float dt, LineRenderer* linerenderer);
    void foo(World* world);
    
    Manifold manifolds[1024];
    uint32_t numManifolds;
};
