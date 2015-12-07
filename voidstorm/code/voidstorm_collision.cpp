#define DBVT_NODE_POOL_SIZE 4096
#define DBVT_STACK_SIZE 4096

Dbvt::Dbvt(void* mem)
	:
	root(nullptr),
	allocator(mem, sizeof(DbvtNode), DBVT_NODE_POOL_SIZE)
{}

DbvtNode* Dbvt::createProxy(Entity entity, const AABB& aabb)
{
    DbvtNode* node = new(allocator.alloc(0)) DbvtNode(entity, aabb);

    insertNode(node);
    return node;
}

void Dbvt::destroyProxy(DbvtNode* node)
{
    removeNode(node);
    allocator.free(node);
}

void Dbvt::moveProxy(DbvtNode* node, const glm::vec2& displacemnt)
{
    removeNode(node);

    node->aabb.lower += displacemnt;
    node->aabb.upper += displacemnt;

    insertNode(node);
}

void Dbvt::removeNode(DbvtNode* nodeToRemove)
{
    if(root == nodeToRemove)
    {
	root = nullptr;
	return;
    }

    DbvtNode* parent = nodeToRemove->parent;
    DbvtNode* grandParent = parent->parent;
    DbvtNode* sibling;

    if(parent->child1 == nodeToRemove)
    {
	sibling = parent->child2;
    }
    else
    {
	sibling = parent->child1;
    }

    if(grandParent != nullptr)
    {
	if(grandParent->child1 == parent)
	{
	    grandParent->child1 = sibling;
	}
	else
	{
	    grandParent->child2 = sibling;
	}

	sibling->parent = grandParent;
	allocator.free(parent);

	// Traverse the tree resizing the parents.
	DbvtNode* currentNode = grandParent;
	while(currentNode != nullptr)
	{
	    DbvtNode* child1 = currentNode->child1;
	    DbvtNode* child2 = currentNode->child2;

	    currentNode->aabb.combine(child1->aabb, child2->aabb);
	    currentNode = currentNode->parent;
	} 
	
    }
    else
    {
	root = sibling;
	sibling->parent = nullptr;
	allocator.free(parent);
    }
}

void Dbvt::insertNode(DbvtNode* nodeToInsert)
{
    if(root == nullptr)
    {
	root = nodeToInsert;
	root->parent = nullptr;
	return;
    }

    AABB nodeToInsertAABB = nodeToInsert->aabb;
    DbvtNode* currentNode = root;
    while(!currentNode->isLeaf())
    {
	float area = currentNode->aabb.getPerimiter();

	AABB combinedAABB;
	combinedAABB.combine(currentNode->aabb, nodeToInsertAABB);

	float combinedArea = combinedAABB.getPerimiter();

	float cost = 2.0f * combinedArea;

	float inheritanceCost = 2.0f* (combinedArea - area);
	
	float cost1 = 0.0f;
	float cost2 = 0.0f;

	if(currentNode->child1->isLeaf())
	{
	    AABB aabb;
	    aabb.combine(nodeToInsertAABB, currentNode->child1->aabb);
	    cost1 = aabb.getPerimiter() + inheritanceCost;
	}
	else
	{
	    AABB aabb;
	    aabb.combine(nodeToInsertAABB, currentNode->child1->aabb);
	    float oldArea = currentNode->child1->aabb.getPerimiter();
	    float newArea = aabb.getPerimiter();
	    cost1 = (newArea - oldArea) + inheritanceCost;
	}
	
	if(currentNode->child2->isLeaf())
	{
	    AABB aabb;
	    aabb.combine(nodeToInsertAABB, currentNode->child2->aabb);
	    cost2 = aabb.getPerimiter() + inheritanceCost;	    
	}
	else
	{
	    AABB aabb;
	    aabb.combine(nodeToInsertAABB, currentNode->child2->aabb);
	    float oldArea = currentNode->child2->aabb.getPerimiter();
	    float newArea = aabb.getPerimiter();
	    cost1 = (newArea - oldArea) + inheritanceCost;
	}

	if(cost < cost1 && cost < cost2)
	{
	    break;
	}
	
	if(cost1 < cost2)
	{
	    currentNode = currentNode->child1;
	}
	else
	{
	    currentNode = currentNode->child2;
	}	
    }

    DbvtNode* sibling = currentNode;
    
    DbvtNode* oldParent = sibling->parent;

    AABB newParentAABB;
    newParentAABB.combine(nodeToInsertAABB, sibling->aabb);

    DbvtNode* newParent = new(allocator.alloc(0)) DbvtNode(Entity_Null, newParentAABB);
    newParent->parent = oldParent;

    if(oldParent != nullptr)
    {
	if(oldParent->child1 == sibling)
	{
	    oldParent->child1 = newParent;
	}
	else
	{
	    oldParent->child2 = newParent;
	}
	
	newParent->child1 = sibling;
	newParent->child2 = nodeToInsert;
	
	sibling->parent = newParent;
	nodeToInsert->parent = newParent;
    }
    else
    {
	newParent->child1 = sibling;
	newParent->child2 = nodeToInsert;

	sibling->parent = newParent;
	nodeToInsert->parent = newParent;

	// The sibling was the root.
	root = newParent;
    }

    // Traverse the tree resizing the parents.
    currentNode = nodeToInsert->parent;
    while(currentNode != nullptr)
    {
	DbvtNode* child1 = currentNode->child1;
	DbvtNode* child2 = currentNode->child2;

	currentNode->aabb.combine(child1->aabb, child2->aabb);
	currentNode = currentNode->parent;
    }    
}

void Dbvt::visualize(LineRenderer* lineRenderer)
{
    DbvtNode* stack[DBVT_STACK_SIZE];
    int count = 0;
    stack[count++] = root;

    while(count > 0)
    {
	DbvtNode* node = stack[--count];

	if(node == nullptr)
	    continue;
	
	lineRenderer->drawAABB(node->aabb, glm::vec4(1,1,0,1));

	stack[count++] = node->child1;
	stack[count++] = node->child2;
    }
}

int Dbvt::query(const AABB& aabb, Contact* contacts)
{
    DbvtNode* stack[DBVT_STACK_SIZE];
    int count = 0;
    stack[count++] = root;

    int contactCounter = 0;
    
    while(count > 0)
    {
	DbvtNode* current = stack[--count];

	if(current == nullptr)
	    continue;
	
	if(aabbOverlap(current->aabb, aabb))
	{
	    if(current->isLeaf())
	    {
		Contact* c = contacts + contactCounter;
		c->entity = current->entity;
		contactCounter++;
	    }
	    else
	    {
		stack[count++] = current->child1;
		stack[count++] = current->child2;
	    }
	}	
    }

    return contactCounter;
}

int Dbvt::query(DbvtNode* node, Contact* contacts)
{
    DbvtNode* stack[DBVT_STACK_SIZE];
    int count = 0;
    stack[count++] = root;

    int contactCounter = 0;
    
    while(count > 0)
    {
	DbvtNode* current = stack[--count];

	if(current == nullptr)
	    continue;
	
	if(aabbOverlap(current->aabb, node->aabb))
	{
	    if(current->isLeaf())
	    {
		if(node != current)
		{
		    Contact* c = contacts + contactCounter;
		    c->entity = current->entity;
 		    contactCounter++;
		}
	    }
	    else
	    {
		stack[count++] = current->child1;
		stack[count++] = current->child2;
	    }
	}	
    }

    return contactCounter;
}

CollisionManager::CollisionManager(dcutil::StackAllocator* _stack)
	: stack(_stack), tree(stack->alloc(sizeof(DbvtNode) * DBVT_NODE_POOL_SIZE))
{

}

void CollisionManager::allocate(uint32_t count)
{
    size_t size = count * (sizeof(Entity)
			   + 2 * sizeof(uint32_t)
			   + sizeof(ShapeData)
			   + sizeof(DbvtNode*)
			   + sizeof(Contact));

    data.data = stack->alloc(size);
    data.count = count;
    data.used = 1;

    data.entities = (Entity*)data.data;
    data.type = (uint32_t*)(data.entities + count);
    data.mask = (uint32_t*)(data.type + count);
    data.shape = (ShapeData*)(data.mask + count);
    data.node = (DbvtNode**)(data.shape + count);
    data.contact = (Contact*)(data.node + count);
}

CollisionManager::Instance CollisionManager::create(Entity e, uint32_t type, uint32_t mask)
{
    int compIndex = data.used++;
    map.add(stack, e, compIndex);
    data.entities[compIndex] = e;
    data.type[compIndex] = type;
    data.mask[compIndex] = mask;
    data.shape[compIndex].offset = glm::vec2(0, 0);
    data.shape[compIndex].shape = ShapeType::NONE;
    data.node[compIndex] = nullptr;
    
    data.contact[compIndex].entity = Entity_Null;
    data.contact[compIndex].next = nullptr;
    
    //PRINT("CollisionComponent created for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", compIndex);
    //PRINT("Components used: %d\n", data.used - 1);
    
    return { compIndex };
}

void CollisionManager::destroy(Instance i)
{
    if(i.index == 0)
	return;

    uint32_t lastIndex = data.used - 1;
    Entity e = data.entities[i.index];
    Entity last_e = data.entities[lastIndex];

    data.entities[i.index] = data.entities[lastIndex];
    data.type[i.index] = data.type[lastIndex];
    data.mask[i.index] = data.mask[lastIndex];
    data.shape[i.index].offset = data.shape[lastIndex].offset;

    if(data.shape[i.index].shape != ShapeType::NONE)
	g_allocator->free(data.shape[i.index].data.ptr);

    data.shape[i.index].shape = data.shape[lastIndex].shape;
    data.shape[i.index].data = data.shape[lastIndex].data;

    tree.destroyProxy(data.node[i.index]);
    
    data.node[i.index] = data.node[lastIndex];
    
    data.contact[i.index].entity = data.contact[lastIndex].entity;
    data.contact[i.index].next = data.contact[lastIndex].next;
    
    map.remove(e);
    map.remove(last_e);
    map.add(stack, last_e, i.index);

    data.used--;

    //PRINT("CollisionComponent destroyed for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", i.index);
    //PRINT("Components used: %d\n", data.used - 1);
}

void CollisionManager::addContacts(Instance i)
{
   // Query for contacts
    Contact contacts[1024];
    int count = tree.query(data.node[i.index], contacts);
    
    Contact* contact = &data.contact[i.index];
    
    for(int index = 0; index < count; ++index)
    {
	Contact* otherContact = contacts + index;
	do
	{
	    if(contact->entity == otherContact->entity)
	    {
		break;
	    }

	    if(contact->entity != Entity_Null && !contact->next)
	    {
		Contact* newContact = (Contact*)stack->alloc(sizeof(Contact));
		newContact->entity = Entity_Null;
		newContact->next = nullptr;

		contact->next = newContact;
	    }

	    if(contact->entity == Entity_Null)
	    {
		contact->entity = otherContact->entity;

		ShapeType shapeA = data.shape[i.index].shape;
		
		Instance instance = lookup(otherContact->entity);
		ShapeType shapeB = data.shape[instance.index].shape;

		contact->callback = g_contactCallbacks[shapeA][shapeB];
		assert(contact->callback != NULL);

		break;
	    }
	
	    contact = contact->next;

	} while(contact);
    }
}

void CollisionManager::resetContacts(Instance i)
{
    Contact* contact = &data.contact[i.index];
    while(contact)
    {
	contact->entity = Entity_Null;
	contact = contact->next;
    }
}

void CollisionManager::createCircleShape(Instance i, float radius, const glm::vec2& position)
{
    // Allocate shape
    if(data.shape[i.index].shape != ShapeType::NONE)
	g_allocator->free(data.shape[i.index].data.ptr);
    
    data.shape[i.index].shape = ShapeType::CIRCLE;
    data.shape[i.index].data.ptr = g_allocator->alloc(sizeof(CircleShape));
    data.shape[i.index].data.circle->radius = radius;

    // Build AABB that covers the collision shape	    
    AABB aabb;
    aabb.lower = glm::vec2(position.x - radius, position.y - radius);
    aabb.upper = glm::vec2(position.x + radius, position.y + radius);

    // Insert into tree
    data.node[i.index] = tree.createProxy(data.entities[i.index], aabb);

    addContacts(i);	     
}

void CollisionManager::createPolygonShape(Instance i, glm::vec2* vertices, uint32_t count)
{
    // Allocate shape
    if(data.shape[i.index].shape != ShapeType::NONE)
	g_allocator->free(data.shape[i.index].data.ptr);
    
    data.shape[i.index].shape = ShapeType::POLYGON;
    data.shape[i.index].data.ptr = g_allocator->alloc(sizeof(PolygonShape));

    data.shape[i.index].data.polygon->set(vertices, count);

    AABB aabb = data.shape[i.index].data.polygon->computeAABB();

    // Insert into tree
    data.node[i.index] = tree.createProxy(data.entities[i.index], aabb);

    addContacts(i);
}

void CollisionManager::setRadius(Instance i, float radius)
{
    assert(data.shape[i.index].shape == ShapeType::CIRCLE);
    data.shape[i.index].data.circle->radius = radius;
}
    
CONTACT_CALLBACK* g_contactCallbacks[ShapeType::NONE][ShapeType::NONE];

static ContactResult circleVsCircle(World* world, glm::vec2 deltaVel, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB)
{
    ContactResult result;

    glm::vec2 offset = shapeA.offset;
    glm::vec2 otherOffset = shapeB.offset;
    
    CircleShape* circleA = shapeA.data.circle;
    CircleShape* circleB = shapeB.data.circle;

    glm::vec2 pos = world->transforms.data.position[transformA.index];
    glm::vec2 otherPos = world->transforms.data.position[transformB.index];
    
    float scale = world->transforms.data.scale[transformA.index];
    float otherScale = world->transforms.data.scale[transformB.index];
    
    glm::vec2 center = pos + offset + deltaVel;
    float scaledRadius = circleA->radius * scale;

    glm::vec2 otherCenter = otherPos + otherOffset;
    float otherScaledRadius = circleB->radius * otherScale;

    float distance = glm::distance(center, otherCenter);
    if(distance < (scaledRadius + otherScaledRadius))
    {
	float t = (scaledRadius + otherScaledRadius) - distance;
	
	result.hit = true;			        
	result.normal = glm::normalize(center - otherCenter);
	result.position = center - t * result.normal;	
    }
    
    return result;
}

static ContactResult circleVsPolygon(World* world, glm::vec2 deltaVel, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB)
{
    ContactResult result;

    glm::vec2 offset = shapeA.offset;
    glm::vec2 otherOffset = shapeB.offset;

    CircleShape* circle = shapeA.data.circle;
    PolygonShape* polygon = shapeB.data.polygon;
    
    glm::vec2 pos = world->transforms.data.position[transformA.index];
    glm::vec2 otherPos = world->transforms.data.position[transformB.index];

    glm::vec2 center = pos + offset + deltaVel;
    glm::vec2 otherCenter = otherPos + otherOffset;

    int normalIndex = 0;
    float separation = -FLT_MAX;
    
    for(int i = 0; i < polygon->count; ++i)
    {
	float s = glm::dot(polygon->normals[i], center - polygon->vertices[i]);

	if(s > circle->radius)
	{
	    // Early out
	    return result;
	}

	if(s > separation)
	{
	    separation = s;
	    normalIndex = i;
	}
    }

    // Vertices that subtend the incident face
    int vertexIndex0 = normalIndex;
    int vertexIndex1 = vertexIndex0 + 1 < polygon->count ? vertexIndex0 + 1 : 0;
    glm::vec2 v0 = polygon->vertices[vertexIndex0];
    glm::vec2 v1 = polygon->vertices[vertexIndex1];

    // Compute barycentric coordinates
    float u0 = glm::dot(center - v0, glm::normalize(v1 - v0));
    float u1 = glm::dot(center - v1, glm::normalize(v0 - v1));
    
    // Center is inside the polygon
    if(separation < VOIDSTORM_EPSILON)
    {
	result.hit = true;
	result.normal = polygon->normals[normalIndex];
	result.position = (v0 + u0 * glm::normalize(v1 - v0)) + result.normal * circle->radius;
	return result;
    }    

    if(u0 <= 0.0f)
    {
	if(glm::distance(center, v0) > circle->radius)
	{
	    return result;
	}

	result.hit = true;
	result.normal = glm::normalize(center - v0);
	result.position = v0 + result.normal * circle->radius;
    }
    else if(u1 <= 0.0f)
    {
	if(glm::distance(center, v1) > circle->radius)
	{
	    return result;
	}

	result.hit = true;
	result.normal = glm::normalize(center - v1);
	result.position = v1 + result.normal * circle->radius;
    }
    else
    {
	glm::vec2 faceCenter = 0.5f * (v0 + v1);
	float sep = glm::dot(center - faceCenter, polygon->normals[vertexIndex0]);
	if(sep > circle->radius)
	{
	    return result;

	}
	
	result.hit = true;
	result.normal = polygon->normals[vertexIndex0];
	result.position = (v0 + u0 * glm::normalize(v1 - v0)) + result.normal * circle->radius;
    }
    
    return result;
}

static ContactResult stub(World* world, glm::vec2 deltaVel, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB)
{
    ContactResult result;
    return result;
}

void setupContactCallbacks()
{
    g_contactCallbacks[ShapeType::CIRCLE][ShapeType::CIRCLE] = circleVsCircle;
    g_contactCallbacks[ShapeType::POLYGON][ShapeType::POLYGON] = stub;
    g_contactCallbacks[ShapeType::CIRCLE][ShapeType::POLYGON] = circleVsPolygon;
    g_contactCallbacks[ShapeType::POLYGON][ShapeType::CIRCLE] = circleVsPolygon;
}
