#pragma once

struct Contact;

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
class LineRenderer;
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

struct CollisionManager
{
    CollisionManager(dcutil::StackAllocator* _stack);

    void reset();
    
    struct Instance { int index; };

    struct ShapeData
    {
	glm::vec2 offset;
	ShapeType shape;
	
	union
	{
	    void* ptr;
	    PolygonShape* polygon;
	    CircleShape* circle;
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
    
    void setRadius(Instance i, float radius);

    dcutil::StackAllocator* stack;
    ComponentMap map;
    Dbvt tree;
};

struct ContactResult
{
    ContactResult()
	    : hit(false) {}
    
    bool hit;
    glm::vec2 normal;
    glm::vec2 position;    
    float r;
};

struct World;
typedef ContactResult CONTACT_CALLBACK(World* world,
				       glm::vec2 deltaVel,
				       TransformManager::Instance transformA,
				       CollisionManager::ShapeData shapeA,
				       TransformManager::Instance transformB,
				       CollisionManager::ShapeData shapeB);

extern CONTACT_CALLBACK* g_contactCallbacks[ShapeType::NONE][ShapeType::NONE];

void setupContactCallbacks();

struct Contact
{
    Entity entity;
    CONTACT_CALLBACK* callback;
    Contact* next;
};

