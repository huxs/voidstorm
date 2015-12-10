void ComponentMap::add(dcutil::StackAllocator* stack, Entity entity, int compId)
{
    int hash = dcutil::sdbm32((char*)&entity.id, sizeof(entity.id));
    int slot = hash & (ARRAYSIZE(map)-1);

    //PRINT("ADD Entity %d, CompID %d :  with hash %d at slot %d\n", entity.id, compId, hash, slot);
    
    Node* node = map + slot;
    do
    {
	if(node->value == compId) // If entity-comp mapping already exist.
	{
	    node->entity = entity;
	    break;
	}
	
	if(node->value != NODE_UNINITIALIZED && !node->next) // Node exist but next does not, extend the internal chain.
	{
	    node->next = (Node*)stack->alloc(sizeof(Node));

	    node = node->next;	    
	    node->value = NODE_UNINITIALIZED;
	    node->next = nullptr;
	}

	if(node->value == NODE_UNINITIALIZED) // Empty node, write the entity-comp mapping.
	{
	    node->entity = entity;
	    node->value = compId;	    
	    break;
	}

	node = node->next;
	
    } while(node);
}

void ComponentMap::remove(Entity entity)
{
    int hash = dcutil::sdbm32((char*)&entity.id, sizeof(entity.id));
    int slot = hash & (ARRAYSIZE(map)-1);

    Node* node = map + slot;
    do
    {
	if(node->entity.id == entity.id)
	{
	    node->entity = { ENTITY_INVALID };
	    node->value = NODE_UNINITIALIZED;
	    break;
	}
	
	node = node->next;
	
    } while(node);
}

int ComponentMap::lookup(Entity entity)
{
    int hash = dcutil::sdbm32((char*)&entity.id, sizeof(entity.id));
    int slot = hash & (ARRAYSIZE(map)-1);

    int value = 0;
    Node* node = map + slot;
    do
    {
	if (node == nullptr)
	    break;

	if(node->entity.id == entity.id)
	{
	    value = (node->value == NODE_UNINITIALIZED) ? 0 : node->value;
	    break;
	}
	
	node = node->next;
	
    } while(node);

    return value;
}

void TransformManager::allocate(uint32_t count)
{
    size_t size = count * (sizeof(Entity) + sizeof(glm::vec2) + sizeof(float) + sizeof(float) + sizeof(float));
    
    data.data = g_permStackAllocator->alloc(size);
    data.count = count;
    data.used = 1;

    data.entities = (Entity*)data.data;
    data.position = (glm::vec2*)(data.entities + count);
    data.rotation = (float*)(data.position + count);
    data.depth = (float*)(data.rotation + count);
    data.scale = (float*)(data.depth + count);
}

TransformManager::Instance TransformManager::create(Entity e)
{
    int compIndex = data.used++;	
    map.add(g_permStackAllocator, e, compIndex);
    
    data.entities[compIndex] = e;
    data.position[compIndex] = glm::vec2(0, 0);
    data.rotation[compIndex] = 0.0f;
    data.depth[compIndex] = 1.0f;
    data.scale[compIndex] = 1.0f;

    //PRINT("TransformComponent created for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", compIndex);
    //PRINT("Components used: %d\n", data.used - 1);
    
    return { compIndex };	
}

void TransformManager::destroy(Instance i)
{
    if(i.index == 0)
	return;
    
    uint32_t lastIndex = data.used - 1;
    Entity e = data.entities[i.index];
    Entity last_e = data.entities[lastIndex];
	
    data.entities[i.index] = data.entities[lastIndex];
    data.position[i.index] = data.position[lastIndex];
    data.rotation[i.index] = data.rotation[lastIndex];
    data.depth[i.index] = data.depth[lastIndex];
    data.scale[i.index] = data.scale[lastIndex];

    map.remove(e);
    map.remove(last_e);
    map.add(g_permStackAllocator, last_e, i.index);

    data.used--;

    //PRINT("TransformComponent destroyed for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", i.index);
    //PRINT("Components used: %d\n", data.used - 1);
}

void PhysicsManager::allocate(uint32_t count)
{
    size_t size = count * (sizeof(Entity) +  sizeof(float) + sizeof(glm::vec2)
			   + sizeof(glm::vec2) + sizeof(glm::vec2));
    
    data.data  = g_permStackAllocator->alloc(size);
    data.count = count;
    data.used = 1;

    data.entities = (Entity*)data.data;
    data.mass = (float*)(data.entities + count);
    data.force = (glm::vec2*)(data.mass + count);
    data.acceleration = (glm::vec2*)(data.force + count);
    data.velocity = (glm::vec2*)(data.acceleration + count);

};

PhysicsManager::Instance PhysicsManager::create(Entity e)
{
    int compIndex = data.used++;	
    map.add(g_permStackAllocator, e, compIndex);
    data.entities[compIndex] = e;
    data.mass[compIndex] = 1.0f;
    data.force[compIndex] = glm::vec2(0, 0);
    data.acceleration[compIndex] = glm::vec2(0, 0);
    data.velocity[compIndex] = glm::vec2(0, 0);
    

    //PRINT("PhysicsComponent created for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", compIndex);
    //PRINT("Components used: %d\n", data.used - 1);
    
    return { compIndex };	
}

void PhysicsManager::destroy(Instance i)
{
    if(i.index == 0)
	return;
    
    uint32_t lastIndex = data.used - 1;
    Entity e = data.entities[i.index];
    Entity last_e = data.entities[lastIndex];
	
    data.entities[i.index] = data.entities[lastIndex];
    data.mass[i.index] = data.mass[lastIndex];
    data.force[i.index] = data.force[lastIndex];
    data.acceleration[i.index] = data.acceleration[lastIndex];
    data.velocity[i.index] = data.velocity[lastIndex];

    map.remove(e);
    map.remove(last_e);
    map.add(g_permStackAllocator, last_e, i.index);
	
    data.used--;

    //PRINT("PhysicsComponent destroyed for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", i.index);
    //PRINT("Components used: %d\n", data.used - 1);
}

void CollisionResponderManager::allocate(uint32_t count)
{
    size_t size = count * (sizeof(Entity) + sizeof(CollisionResult));

    data.data = g_permStackAllocator->alloc(size);
    data.count = count;
    data.used = 1;

    data.entities = (Entity*)data.data;
    data.collidedWith = (CollisionResult*)(data.entities + count);
}

CollisionResponderManager::Instance CollisionResponderManager::create(Entity e)
{
    int compIndex = data.used++;
    map.add(g_permStackAllocator, e, compIndex);
    data.entities[compIndex] = e;

    CollisionResult r;
    for(int i = 0; i < ARRAYSIZE(r.entity); ++i)
	r.entity[i] = { UINT32_MAX };
    
    data.collidedWith[compIndex] = r;

    //PRINT("CollisionResponderComponent created for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", compIndex);
    //PRINT("Components used: %d\n", data.used - 1);
    
    return { compIndex };
}

void CollisionResponderManager::destroy(Instance i)
{    
    if(i.index == 0)
	return;
    
    uint32_t lastIndex = data.used - 1;
    Entity e = data.entities[i.index];
    Entity last_e = data.entities[lastIndex];

    data.entities[i.index] = data.entities[lastIndex];
    data.collidedWith[i.index] = data.collidedWith[lastIndex];

    map.remove(e);
    map.remove(last_e);
    map.add(g_permStackAllocator, last_e, i.index);

    data.used--;
    
    //PRINT("CollisionResponderComponent destroyed for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", i.index);
    //PRINT("Components used: %d\n", data.used);
}


void SpriteManager::allocate(uint32_t count)
{
    size_t size = count * (sizeof(Entity) + sizeof(glm::vec4) + sizeof(Texture*) + sizeof(glm::vec2) + sizeof(glm::vec2));
    
    data.data  = g_permStackAllocator->alloc(size);
    data.count = count;
    data.used = 1;

    data.entities = (Entity*)data.data;
    data.color = (glm::vec4*)(data.entities + count);
    data.texture = (Texture**)(data.color + count);
    data.size = (glm::vec2*)(data.texture + count);
    data.origin = (glm::vec2*)(data.size + count);
};

SpriteManager::Instance SpriteManager::create(Entity e)
{
    int compIndex = data.used++;	
    map.add(g_permStackAllocator, e, compIndex);
	
    data.entities[compIndex] = e;
    data.color[compIndex] = glm::vec4(1, 1, 1, 1);
    data.texture[compIndex] = nullptr;
    data.origin[compIndex] = glm::vec2(0.5, 0.5);

    //PRINT("SpriteComponent created for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", compIndex);
    //PRINT("Components used: %d\n", data.used - 1);
    
    return { compIndex };	
}

void SpriteManager::destroy(SpriteManager::Instance i)
{
    if(i.index == 0)
	return;

    uint32_t lastIndex = data.used - 1;
    Entity e = data.entities[i.index];
    Entity last_e = data.entities[lastIndex];
	
    data.entities[i.index] = data.entities[lastIndex];
    data.texture[i.index] = data.texture[lastIndex];
    data.color[i.index] = data.color[lastIndex];
    data.size[i.index] = data.size[lastIndex];
    data.origin[i.index] = data.origin[lastIndex];
    
    map.remove(e);
    map.remove(last_e);
    map.add(g_permStackAllocator, last_e, i.index);

    data.used--;

    //PRINT("SpriteComponent destroyed for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", i.index);
    //PRINT("Components used: %d\n", data.used - 1);
}
