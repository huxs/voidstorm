#pragma once

#define NODE_UNINITIALIZED UINT32_MAX

struct Node
{
    Node() : entity({ENTITY_INVALID}), value(NODE_UNINITIALIZED), next(0) {}

    Entity entity;
    int value;
    Node* next;
};

struct ComponentMap
{
    void add(dcutil::StackAllocator* stack, Entity entity, int compId);
    void remove(Entity entity);
    int lookup(Entity entity);
    
    Node map[100];
};

struct TransformManager
{
    TransformManager(dcutil::StackAllocator* _stack)
	    : stack(_stack) {}
    
    struct Instance { int index; };
	
    struct Data
    {
	uint32_t count;
	uint32_t used;
	void* data;		
	Entity* entities;		
	glm::vec2* position;
	float* rotation;
	float* depth;
	float* scale;
	
    } data;
	
    void allocate(uint32_t count);
    Instance create(Entity e);
    void destroy(Instance i);
    Instance lookup(Entity e) { return { map.lookup(e) }; };
 	 
    void setPosition(Instance i, glm::vec2 position) { data.position[i.index] = position; }
    glm::vec2 getPosition(Instance i) { return data.position[i.index]; }
    void setRotation(Instance i, float rotation) { data.rotation[i.index] = rotation; }
    float getRotation(Instance i) { return data.rotation[i.index]; }
    void setDepth(Instance i, float depth) { data.depth[i.index] = depth;}
    float getDepth(Instance i) { return data.depth[i.index]; }
    void setScale(Instance i, float scale) { data.scale[i.index] = scale;}
    float getScale(Instance i) { return data.scale[i.index]; }
    
    dcutil::StackAllocator* stack;
    ComponentMap map;
};

struct PhysicsManager
{
    PhysicsManager(dcutil::StackAllocator* _stack)
	    : stack(_stack) {}
    
    struct Instance { int index; };

    struct Data
    {
	uint32_t count;
	uint32_t used;
	void* data;
	Entity* entities;
	float* mass;
	glm::vec2* force;
	glm::vec2* acceleration;
	glm::vec2* velocity;		
    } data;

    void allocate(uint32_t count);
    Instance create(Entity e);
    void destroy(Instance i);
    Instance lookup(Entity e) { return { map.lookup(e) }; };

    void setMass(Instance i, float mass) { data.mass[i.index] = mass; }
    void addForce(Instance i, glm::vec2 force) { data.force[i.index] = force; }

    dcutil::StackAllocator* stack;
    ComponentMap map;
};

struct CollisionResponderManager
{
    CollisionResponderManager(dcutil::StackAllocator* _stack)
	    : stack(_stack) {}
    
    struct Instance { int index; };

    struct CollisionResult
    {
	CollisionResult() : entityCount(0) {}

	int entityCount;

	// TODO (daniel): Decide on max number of collisions per frame
	Entity entity[5];
	glm::vec2 position[5];
    };
    
    struct Data
    {
	uint32_t count;
	uint32_t used;
	void* data;
	Entity* entities;
	CollisionResult* collidedWith;

    } data;

    void allocate(uint32_t count);
    Instance create(Entity e);
    void destroy(Instance i);
    Instance lookup(Entity e) { return { map.lookup(e) }; };

    dcutil::StackAllocator* stack;
    ComponentMap map;	
};

struct SpriteManager
{
    SpriteManager(dcutil::StackAllocator* _stack)
	    : stack(_stack) {}
    
    struct Instance { int index; };

    struct Data
    {
	uint32_t count;
	uint32_t used;
	void* data;

	Entity* entities;
	glm::vec4* color;
	Texture** texture;
	glm::vec2* size;
	glm::vec2* origin;
		
    } data;

    void allocate(uint32_t count);
    Instance create(Entity e);
    void destroy(Instance i);
    Instance lookup(Entity e) { return { map.lookup(e) }; };

    void setColor(Instance i, glm::vec4 color) { data.color[i.index] = color; }
    glm::vec4 getColor(Instance i) { return data.color[i.index]; }
    void setTexture(Instance i, Texture* handle) { data.texture[i.index] = handle; }
    void setSize(Instance i, glm::vec2 size) { data.size[i.index] = size; }
    void setOrigin(Instance i, glm::vec2 origin) { data.origin[i.index] = origin; }
    
    dcutil::StackAllocator* stack;
    ComponentMap map;
};
