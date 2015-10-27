#pragma once

struct Contact
{
    Entity entity;
    CONTACT_CALLBACK* callback;
    Contact* next;
};

struct DbvtNode
{
    DbvtNode(Entity _entity, const AABB& _aabb)
	    :
	    entity(_entity),
	    aabb(_aabb),
	    parent(nullptr),
   	    child1(nullptr),
	    child2(nullptr)
	{}
    
	bool isLeaf() { return child1 == nullptr; } const

    Entity entity;
    AABB aabb;
    DbvtNode* parent;    
    DbvtNode* child1;
    DbvtNode* child2;
};

// Dynamic bounding volume tree
class Dbvt
{
public:
    Dbvt(void* mem);
    
    DbvtNode* createProxy(Entity entity, const AABB& aabb);
    void destroyProxy(DbvtNode* node);
    void moveProxy(DbvtNode* node, const glm::vec2& displacemnt);

    int query(const AABB& aabb, Contact* contacts);  
    int query(DbvtNode* node, Contact* contacts);
 
    void visualize(LineRenderer* lineRenderer);
    AABB getTreeAABB() { return root->aabb; }
    
private:
    void insertNode(DbvtNode* nodeToInsert);
    void removeNode(DbvtNode* nodeToRemove);
    
    DbvtNode* root;    
    dcutil::PoolAllocator allocator;
};

class PhysicsWorld
{
public:
    PhysicsWorld(dcutil::StackAllocator* stack, LineRenderer* linerenderer);

    Dbvt* getTree() { return &tree; }
    
    void simulate(World* world, float dt);
    
private:
    LineRenderer* linerenderer;
    Dbvt tree;
    Contact* contacts;
};
