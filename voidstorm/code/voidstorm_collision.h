#pragma once

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

struct CollisionManager
{
    CollisionManager();

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
	DbvtNode** node; // Reference to the physics data structure
	Contact* contact; // Single linked list of contacts
	
    } data;

    void allocate(uint32_t count);
    Instance create(Entity e,  uint32_t type, uint32_t mask);
    void destroy(Instance i);
    Instance lookup(Entity e) { return { map.lookup(e) }; };

    void setType(Instance i, uint32_t type) { data.type[i.index] = type; }
    uint32_t getType(Instance i) { return data.type[i.index]; }
    void setMask(Instance i, uint32_t mask) { data.mask[i.index] = mask; }
    void setOffset(Instance i, const glm::vec2& offset) { data.shape[i.index].offset = offset; }

    void setNode(Instance i, DbvtNode* node) { data.node[i.index] = node; }
    DbvtNode* getNode(Instance i) { return data.node[i.index]; }

    void addContacts(Instance i);
    void resetContacts(Instance i);

    void createCircleShape(Instance i, float radius, const glm::vec2& position);
    void createPolygonShape(Instance i, glm::vec2* vertices, uint32_t count);
    void createRayShape(Instance i, const glm::vec2& direction, const glm::vec2& position);

    void* allocateShape(ShapeType type);
    void freeShape(ShapeType type, void* ptr);
    
    void setRadius(Instance i, float radius);
    void setDirection(Instance i, const glm::vec2& direction, const glm::vec2& position);
    
    ComponentMap map;
    Dbvt tree;
    dcutil::PoolAllocator circlePool;
    dcutil::PoolAllocator polygonPool;
    dcutil::PoolAllocator rayPool;;
};

struct ContactResultEx
{
    glm::vec2 position;
    glm::vec2 normal;
};

struct Manifold
{
    Manifold() : numContacts(0) {}
    
    ContactResultEx contacts[2];
    uint8_t numContacts;
 	
    TransformManager::Instance instA;
    TransformManager::Instance instB;
    PhysicsManager::Instance pinstA;
    PhysicsManager::Instance pinstB;
};

struct World;
typedef Manifold CONTACT_CALLBACK(World* world,
				       glm::vec2 deltaVel,
				       glm::vec2 otherDeltaVel,
				       TransformManager::Instance transformA,
				       CollisionManager::ShapeData shapeA,
				       TransformManager::Instance transformB,
				       CollisionManager::ShapeData shapeB);

extern CONTACT_CALLBACK* g_contactCallbacks[ShapeType::NONE][ShapeType::NONE];

struct Contact
{
    Entity entity;
    CONTACT_CALLBACK* callback;
    Contact* next;
};

