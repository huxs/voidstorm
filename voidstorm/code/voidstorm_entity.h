#pragma once

// 24 bit id
#define ENTITY_ID_MASK 0x00ffffff

// 8 bit generation
#define ENTITY_GENERATION_MASK 0xff000000
#define ENTITY_GENERATION_SHIFT 24

#define ENTITY_INVALID 0xffffffff

struct Entity
{
    uint32_t id;

    uint32_t index() { return id & ENTITY_ID_MASK; }
    uint32_t generation() { return (id & ENTITY_GENERATION_MASK) >> ENTITY_GENERATION_SHIFT; }

    bool operator==(const Entity& other);
    bool operator!=(const Entity& other);
};

inline bool Entity::operator==(const Entity& other)
{
    return (this->id == other.id);
}

inline bool Entity::operator!=(const Entity& other)
{
    return (this->id != other.id);
}

static const Entity Entity_Null{ ENTITY_INVALID };

class EntityManager
{
public:
    EntityManager() : generationCount(0), freeIndicesCount(0) {}

    void reset();
    
    Entity create();
    void destroy(Entity e);
    bool alive(Entity e);

    int getNrOfEntities();
    
private:
    uint8_t generation[VOIDSTORM_ENTITY_MAX_COUNT];
    uint32_t generationCount;

    uint32_t freeIndices[VOIDSTORM_ENTITY_MAX_COUNT];
    uint32_t freeIndicesCount;
};


