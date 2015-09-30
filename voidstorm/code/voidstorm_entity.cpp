Entity EntityManager::create()
{
    uint32_t index;
    if(freeIndicesCount > 0)
    {
	freeIndicesCount--;
	index = freeIndices[freeIndicesCount];
    }
    else
    {
	assert(generationCount < ARRAYSIZE(generation));
	generation[generationCount] = 0;
	index = generationCount;
	generationCount++;
	assert(index < (1 << ENTITY_GENERATION_SHIFT));
    }

    Entity e = { ((generation[index] << ENTITY_GENERATION_SHIFT) & ENTITY_GENERATION_MASK) | index };
    PRINT("Entity created!\n");
    PRINT("Index: %d\n", e.index());
    PRINT("Generation: %d\n", e.generation());
    
    return e;
}

void EntityManager::destroy(Entity e)
{
    uint32_t index = e.index();
	
    generation[index]++;	

    assert(freeIndicesCount < ARRAYSIZE(freeIndices));
    freeIndices[freeIndicesCount] = index;
    freeIndicesCount++;
    
    PRINT("Entity destroyed!\n");
    PRINT("Index: %d\n", e.index());
    PRINT("Generation: %d\n", e.generation());
}

bool EntityManager::alive(Entity e)
{
    return (e.generation() == generation[e.index()]);
}

int EntityManager::getNrOfEntities()
{
    return generationCount - freeIndicesCount;
}
