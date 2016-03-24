#pragma once

/*
  The collision system works as following:

  The collision component contains three vital pieces of information:
  1) The shape of the entity (Ray, Circle, Polygon..)
  2) Pointer to the DBTV (Dynamic Bounding Volume Tree) Node identifying the entity
     in the broad phase collision detection (AABB test)
  3) Linked list of entities which intersect during the broad phase test

  A broad phase test is executed for an entity whenever the collision component is created
  or if the entity is transformed by the API or the physics system.

  Once per frame the physics system asks for the contact list of each collision component
  and performs a narrow test if there exist a contact.

  The narrow test generates a manifold from the two colliding entities containing
  the contact point and the impact normal.

  If both the colliding entities have physics components the manifold is added for
  impulse resolution.

  The impulse resolution looks at the manifold and the physical properties of the objects and
  updates the velocities accordingly.

 */

#define DBVT_POOL_MAX_NODES 4096
#define DBVT_QUERY_STACK_MAX_NODES 4096

#define BROADPHASE_MAX_CONTACTS 1024

#define SHAPE_POOL_MAX_CIRCLES 1024
#define SHAPE_POOL_MAX_POLYGONS 1024
#define SHAPE_POOL_MAX_RAYS 1024

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

struct Contact;
class LineRenderer;
class Dbvt
{
public:
    Dbvt(void* mem);
    
    DbvtNode* createProxy(Entity entity, const AABB& aabb);
    void destroyProxy(DbvtNode* node);
    void moveProxy(DbvtNode* node, const glm::vec2& displacemnt);
    void updateProxyBounds(DbvtNode* node, const AABB& bounds);
	
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

class CollisionManager
{
public:
    CollisionManager(dcutil::StackAllocator *_allocator);

    void reset();
    
    struct Instance { uint32_t index; };

    struct ShapeData
    {
	glm::vec2 offset;
	ShapeType shape;
	
	union
	{
	    void* ptr;
	    PolygonShape* polygon;
	    CircleShape* circle;
	    RayShape* ray;
	} data;
    };
    
    struct Data
    {
	uint32_t count;
	uint32_t used;
	void* data;

	Entity* entities;
	uint32_t* type;
	uint32_t* mask;
	ShapeData* shape;	        
	DbvtNode** node; // Reference to the broad phase data structure
	Contact* contact; // Single linked list of contacts
	
    } data;

    void allocate(uint32_t count);
    Instance create(Entity e,  uint32_t type, uint32_t mask);
    void destroy(Instance i);
    Instance lookup(Entity e) { return { map.lookup(e) }; };

    void setType(Instance i, uint32_t type) { data.type[i.index] = type; }
    uint32_t getType(Instance i) { return data.type[i.index]; }
    void setMask(Instance i, uint32_t mask) { data.mask[i.index] = mask; }
    uint32_t getMask(Instance i) { return data.mask[i.index]; }    
    void setOffset(Instance i, const glm::vec2& offset) { data.shape[i.index].offset = offset; }
    void setNode(Instance i, DbvtNode* node) { data.node[i.index] = node; }
    DbvtNode* getNode(Instance i) { return data.node[i.index]; }

    void broadTest(Instance i);

    void createCircleShape(Instance i, float radius, const glm::vec2& position);
    void createPolygonShape(Instance i, glm::vec2* vertices, uint32_t count, const glm::vec2& position);
    void createRayShape(Instance i, const glm::vec2& direction, const glm::vec2& position);

    void setRadius(Instance i, float radius);
    void setDirection(Instance i, const glm::vec2& direction, const glm::vec2& position);
    
    Dbvt tree;

private:
    void* allocateShape(ShapeType type);
    void freeShape(ShapeType type, void* ptr);
    
    dcutil::PoolAllocator circlePool;
    dcutil::PoolAllocator polygonPool;
    dcutil::PoolAllocator rayPool;

    ComponentMap map;
    dcutil::StackAllocator *allocator;
};

struct ContactPoint
{
    glm::vec2 position;
    glm::vec2 normal;
    // NOTE (daniel): Should we store the penetration depth?
};

struct Manifold
{
    Manifold() : numContacts(0) {}

    // NOTE (daniel): Currently we only do impulse resolution from 1 contact point.
    // If we need better resolution we need to update our collision routines
    ContactPoint contacts[1];
    uint8_t numContacts;
 	
    TransformManager::Instance instA;
    TransformManager::Instance instB;
    PhysicsManager::Instance pinstA;
    PhysicsManager::Instance pinstB;
};

// TODO (daniel): Once we converted instances to transforms we can change this
// function signature.
// Also change to use the macro version..
struct World;
typedef Manifold CONTACT_CALLBACK(World* world,
				  TransformManager::Instance transformA,
				  CollisionManager::ShapeData shapeA,
				  TransformManager::Instance transformB,
				  CollisionManager::ShapeData shapeB,
				  LineRenderer* linerender);

extern CONTACT_CALLBACK* g_contactCallbacks[ShapeType::NONE][ShapeType::NONE];

struct Contact
{
    Entity entity;
    CONTACT_CALLBACK* callback;
    Contact* next;
};

