#pragma once

class PhysicsWorld
{
public:
    PhysicsWorld(LineRenderer* linerenderer);

    void simulate(World* world, float dt);
    
private:
    LineRenderer* linerenderer;
};
