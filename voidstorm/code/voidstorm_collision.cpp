Dbvt::Dbvt(void* mem)
	:
	root(nullptr),
	allocator(mem, sizeof(DbvtNode), DBVT_POOL_MAX_NODES)
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

void Dbvt::updateProxyBounds(DbvtNode* node, const AABB& bounds)
{
    removeNode(node);

    node->aabb = bounds;

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
    DbvtNode* stack[DBVT_QUERY_STACK_MAX_NODES];
    int count = 0;
    stack[count++] = root;

    while(count > 0)
    {
	DbvtNode* node = stack[--count];

	if(node == nullptr)
	    continue;
	
	lineRenderer->drawAABB(node->aabb, glm::vec4(1,1,0,1));

	assert(count - 1 < DBVT_QUERY_STACK_MAX_NODES);
	stack[count++] = node->child1;
	stack[count++] = node->child2;
    }
}

int Dbvt::query(const AABB& aabb, Contact* contacts)
{
    DbvtNode* stack[DBVT_QUERY_STACK_MAX_NODES];
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
		assert(contactCounter < BROADPHASE_MAX_CONTACTS);
		Contact* c = contacts + contactCounter;
		c->entity = current->entity;
		contactCounter++;
	    }
	    else
	    {
		assert(count - 1 < DBVT_QUERY_STACK_MAX_NODES);
		stack[count++] = current->child1;
		stack[count++] = current->child2;
	    }
	}	
    }

    return contactCounter;
}

int Dbvt::query(DbvtNode* node, Contact* contacts)
{
    DbvtNode* stack[DBVT_QUERY_STACK_MAX_NODES];
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
		    assert(contactCounter < BROADPHASE_MAX_CONTACTS);
		    Contact* c = contacts + contactCounter;
		    c->entity = current->entity;
 		    contactCounter++;
		}
	    }
	    else
	    {
		assert(count - 1 < DBVT_QUERY_STACK_MAX_NODES);
		stack[count++] = current->child1;
		stack[count++] = current->child2;
	    }
	}	
    }

    return contactCounter;
}
  
CONTACT_CALLBACK* g_contactCallbacks[ShapeType::NONE][ShapeType::NONE];

static Manifold circleVsRay(World* world, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA,
			    TransformManager::Instance transformB, CollisionManager::ShapeData shapeB, LineRenderer* linerender)
{
    Manifold result;
    result.instA = transformA;
    result.instB = transformB;

    // TODO (daniel): Refactor this to use transforms
    
    CircleShape* circle = shapeA.data.circle;
    RayShape* ray = shapeB.data.ray;
    
    glm::vec2 offset = shapeA.offset;
    glm::vec2 otherOffset = shapeB.offset;

    glm::vec2 pos = world->transforms->data.position[transformA.index];
    glm::vec2 otherPos = world->transforms->data.position[transformB.index];
    
    float scale = world->transforms->data.scale[transformA.index];
    glm::vec2 center = pos + offset;
    float radius = circle->radius;
    float radius2 = radius * radius;

    glm::vec2 origin = otherPos + otherOffset;
    glm::vec2 direction = glm::normalize(ray->direction);
    
    glm::vec2 a = center - origin;
    float b = glm::dot(a, direction);
    float c = glm::dot(a,a);

    if(b < 0 && c > radius2)
	return result;

    float m2 = c - b*b;

    if(m2 > radius2)
	return result;

    float q = glm::sqrt(radius2 - m2);
    float t = 0.0f;
    if(c > radius2)
    {
	t = b - q;
    }
    else
    {
	t = b + q;
    }

    glm::vec2 posA = origin + t * direction;
    
    glm::vec2 dir = glm::normalize(posA - center);
    result.contacts[0].normal= glm::normalize(glm::vec2(-(posA.y - origin.y), posA.x - origin.x));

    if(glm::dot(dir, result.contacts[0].normal) > 0)
    {
	result.contacts[0].normal = -result.contacts[0].normal;
    }
    
    result.numContacts = 1;		        
    result.contacts[0].position = posA + result.contacts[0].normal * radius;
    
    return result;
}

static Manifold rayVsCircle(World* world, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB, LineRenderer* linerender)
{
    return circleVsRay(world, transformB, shapeB, transformA, shapeA, linerender);
}

static Manifold circleVsCircle(World* world, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB, LineRenderer* linerender)
{
    Manifold result;
    result.instA = transformA;
    result.instB = transformB;

    // TODO (daniel): Refactor this to use transforms
    
    glm::vec2 offset = shapeA.offset;
    glm::vec2 otherOffset = shapeB.offset;
    
    CircleShape* circleA = shapeA.data.circle;
    CircleShape* circleB = shapeB.data.circle;

    glm::vec2 pos = world->transforms->data.position[transformA.index];
    glm::vec2 otherPos = world->transforms->data.position[transformB.index];
    
    float scale = world->transforms->data.scale[transformA.index];
    float otherScale = world->transforms->data.scale[transformB.index];
    
    glm::vec2 center = pos + offset;
    float scaledRadius = circleA->radius * scale;

    glm::vec2 otherCenter = otherPos + otherOffset;
    float otherScaledRadius = circleB->radius * otherScale;

    float distance = glm::distance(center, otherCenter);
    if(distance < (scaledRadius + otherScaledRadius))
    {
	float t = (scaledRadius + otherScaledRadius) - distance;
	
	result.numContacts = 1;		        
	result.contacts[0].normal = glm::normalize(center - otherCenter);
	result.contacts[0].position = center + t * result.contacts[0].normal;	
    }
    
    return result;
}

// Find the axis where the objects have the max separation
static float findMaxSeparation(int& edgeIndex, PolygonShape* polyA, PolygonShape* polyB, const Transform& tA, const Transform& tB)
{
    int countA = polyA->count;
    int countB = polyB->count;

    Transform t = mulTransT(tB, tA);
    
    int bestIndex = 0;
    float maxSeparation = -FLT_MAX;
    for(int i = 0; i < countA; ++i)
    {
	glm::vec2 normal = polyA->normals[i];
	normal = t.rotate(normal);

	// PolyA position in PolyB frame
	glm::vec2 position = polyA->vertices[i];
	position = t.mul(position);

	float si = FLT_MAX;
	for(int j = 0; j < countB; ++j)
	{
	    float sij = glm::dot(normal, polyB->vertices[j] - position);
	    if(sij < si)
	    {
		si = sij;
	    }
	}

	if(si > maxSeparation)
	{
	    maxSeparation = si;
	    bestIndex = i;
	}
    }

    edgeIndex = bestIndex;
    return maxSeparation;
}

static Manifold polygonVsPolygon(World* world, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA,
				 TransformManager::Instance transformB, CollisionManager::ShapeData shapeB, LineRenderer* linerender)
{
    Manifold result;
    result.numContacts = 0;
    result.instA = transformA;
    result.instB = transformB;

    PolygonShape* polygonA = shapeA.data.polygon;
    PolygonShape* polygonB = shapeB.data.polygon;

    // TODO (daniel): Refactor this block
    glm::vec2 posA = world->transforms->data.position[transformA.index];
    float rotA = world->transforms->data.rotation[transformA.index];
    glm::vec2 posB = world->transforms->data.position[transformB.index];
    float rotB = world->transforms->data.rotation[transformB.index];    
    Transform tA(posA, rotA);
    Transform tB(posB, rotB);
    
    int edgeA = 0;
    float separationA = findMaxSeparation(edgeA, polygonA, polygonB, tA, tB);

    if(separationA > 0.0f)
	return result;
    
    int edgeB = 0;
    float separationB = findMaxSeparation(edgeB, polygonB, polygonA, tB, tA);
    
    if(separationB > 0.0f)
	return result;   

    //PRINT("SepA %f EdgeA %d SepB %f Edge %d\n", separationA, edgeA, separationB, edgeB);

    PolygonShape* poly1; // reference polygon
    PolygonShape* poly2; // incident polygon
    Transform t1;
    Transform t2;
    int refEdge;
    bool flip = false;

    static const float Tolerance = 0.001f;
    
    if(separationB > separationA + Tolerance)
    {
	poly1 = polygonB;
	poly2 = polygonA;
        t1 = tB;
	t2 = tA;
	refEdge = edgeB;
    }
    else
    {
	poly1 = polygonA;
	poly2 = polygonB;
        t1 = tA;
	t2 = tB;
	refEdge = edgeA;
	flip = true;
    }	   

    int a = refEdge;
    int b = refEdge + 1 < poly1->count ? refEdge + 1 : 0;

    glm::vec2 v1 = poly1->vertices[a];
    glm::vec2 v2 = poly1->vertices[b];

    glm::vec2 tangent = v1 - v2;
    tangent = glm::normalize(tangent);

    glm::vec2 normal = glm::vec2(1.0f * tangent.y, -1.0f * tangent.x);
    normal = t1.rotate(normal);

    v1 = t1.mul(v1);
    v2 = t1.mul(v2);
    
    glm::vec2 planePoint = 0.5f * (v1 + v2);

    //linerender->add(v1, v2, glm::vec4(1,0,0,1));
    //linerender->add(planePoint, planePoint + normal * 20.0f, glm::vec4(1,0,0,1));

    result.numContacts = 1;
    result.contacts[0].normal = (flip) ? normal : -normal;
    result.contacts[0].position = planePoint;
    
    return result;
}

static Manifold circleVsPolygon(World* world, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB, LineRenderer* linerender)
{
    Manifold result;
    result.numContacts = 0;
    result.instA = transformA;
    result.instB = transformB;

    CircleShape* circle = shapeA.data.circle;
    PolygonShape* polygon = shapeB.data.polygon;

    // TODO (daniel): Refactor this block
    glm::vec2 posA = world->transforms->data.position[transformA.index];
    float rotA = world->transforms->data.rotation[transformA.index];    
    glm::vec2 posB = world->transforms->data.position[transformB.index];
    float rotB = world->transforms->data.rotation[transformB.index];
    Transform tB(posB, rotB);

    // Computer the circle position in the frame of the polygon
    glm::vec2 center = posA;
    center = tB.mulInverse(center);

    // Find the max separating edge
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
	result.numContacts = 1;
	result.contacts[0].normal = tB.rotate(polygon->normals[normalIndex]);
	result.contacts[0].position = tB.mul((v0 + u0 * glm::normalize(v1 - v0)) + result.contacts[0].normal * circle->radius);
	return result;
    }    

    if(u0 <= 0.0f)
    {
	if(glm::distance(center, v0) > circle->radius)
	{
	    return result;
	}

	result.numContacts = 1;
	result.contacts[0].normal = tB.rotate(glm::normalize(center - v0));
	result.contacts[0].position = tB.mul(v0 + result.contacts[0].normal * circle->radius);
    }
    else if(u1 <= 0.0f)
    {
	if(glm::distance(center, v1) > circle->radius)
	{
	    return result;
	}

	result.numContacts = 1;
	result.contacts[0].normal = tB.rotate(glm::normalize(center - v1));
	result.contacts[0].position = tB.mul(v1 + result.contacts[0].normal * circle->radius);
    }
    else
    {
	glm::vec2 faceCenter = 0.5f * (v0 + v1);
	float sep = glm::dot(center - faceCenter, polygon->normals[vertexIndex0]);
	if(sep > circle->radius)
	{
	    return result;

	}

	result.numContacts = 1;
	result.contacts[0].normal = tB.rotate(polygon->normals[vertexIndex0]);
	result.contacts[0].position = tB.mul((v0 + u0 * glm::normalize(v1 - v0)) + result.contacts[0].normal * circle->radius);
    }
    
    return result;
}

static Manifold polygonVsCircle(World* world, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB, LineRenderer* linerender)
{
    return circleVsPolygon(world, transformB, shapeB, transformA, shapeA, linerender);
}

static Manifold stub(World* world, TransformManager::Instance transformA, CollisionManager::ShapeData shapeA, TransformManager::Instance transformB, CollisionManager::ShapeData shapeB, LineRenderer* linerender)
{
    Manifold result;
    result.numContacts = 0;
    result.instA = transformA;
    result.instB = transformB;
    PRINT("INFO: Collision routine not yet implemented.\n");
    return result;
}

CollisionManager::CollisionManager(dcutil::StackAllocator *_allocator)
	:
	allocator(_allocator),
	tree(_allocator->alloc(sizeof(DbvtNode) * DBVT_POOL_MAX_NODES)),
	circlePool(_allocator->alloc(sizeof(CircleShape) * SHAPE_POOL_MAX_CIRCLES), sizeof(CircleShape), SHAPE_POOL_MAX_CIRCLES),
	polygonPool(_allocator->alloc(sizeof(PolygonShape) * SHAPE_POOL_MAX_POLYGONS), sizeof(PolygonShape), SHAPE_POOL_MAX_POLYGONS),
	rayPool(_allocator->alloc(sizeof(RayShape) * SHAPE_POOL_MAX_RAYS), sizeof(RayShape), SHAPE_POOL_MAX_RAYS)
{
    g_contactCallbacks[ShapeType::CIRCLE][ShapeType::CIRCLE] = circleVsCircle;
    g_contactCallbacks[ShapeType::POLYGON][ShapeType::POLYGON] = polygonVsPolygon;
    g_contactCallbacks[ShapeType::RAY][ShapeType::RAY] = stub;
    
    g_contactCallbacks[ShapeType::CIRCLE][ShapeType::POLYGON] = circleVsPolygon;
    g_contactCallbacks[ShapeType::POLYGON][ShapeType::CIRCLE] = polygonVsCircle;
    g_contactCallbacks[ShapeType::RAY][ShapeType::CIRCLE] = rayVsCircle;
    g_contactCallbacks[ShapeType::CIRCLE][ShapeType::RAY] = circleVsRay;
    g_contactCallbacks[ShapeType::RAY][ShapeType::POLYGON] = stub;
    g_contactCallbacks[ShapeType::POLYGON][ShapeType::RAY] = stub;
}

void CollisionManager::reset()
{
    for(uint32_t i = 1; i < data.used; ++i)
    {
	tree.destroyProxy(data.node[i]);
    }

    circlePool.initialize(sizeof(CircleShape), SHAPE_POOL_MAX_CIRCLES);
    polygonPool.initialize(sizeof(PolygonShape), SHAPE_POOL_MAX_POLYGONS);
    rayPool.initialize(sizeof(RayShape), SHAPE_POOL_MAX_RAYS);

    data.used = 1;
    map = ComponentMap();
}

void CollisionManager::allocate(uint32_t count)
{
    size_t size = count * (sizeof(Entity)
			   + 2 * sizeof(uint32_t)
			   + sizeof(ShapeData)
			   + sizeof(DbvtNode*)
			   + sizeof(Contact));

    data.data = allocator->alloc(size);
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
    uint32_t compIndex = data.used++;
    map.add(allocator, e, compIndex);
    
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

    freeShape(data.shape[i.index].shape, data.shape[i.index].data.ptr);

    data.shape[i.index].shape = data.shape[lastIndex].shape;
    data.shape[i.index].data = data.shape[lastIndex].data;

    tree.destroyProxy(data.node[i.index]);
    
    data.node[i.index] = data.node[lastIndex];
    
    data.contact[i.index].entity = data.contact[lastIndex].entity;
    data.contact[i.index].callback = data.contact[lastIndex].callback;
    data.contact[i.index].next = data.contact[lastIndex].next;
    
    map.remove(e);
    map.remove(last_e);
    map.add(allocator, last_e, i.index);

    data.used--;

    //PRINT("CollisionComponent destroyed for entity I:%d - G:%d!\n", e.index(), e.generation());
    //PRINT("CompId: %d\n", i.index);
    //PRINT("Components used: %d\n", data.used - 1);
}

void CollisionManager::broadTest(Instance i)
{
    // Clear existing contacts
    Contact* contact = &data.contact[i.index];
    while(contact)
    {
	contact->entity = Entity_Null;
	contact = contact->next;
    }
    
    // Query for contacts
    Contact contacts[BROADPHASE_MAX_CONTACTS];
    int count = tree.query(data.node[i.index], contacts);
    
    contact = &data.contact[i.index];
    
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
		Contact* newContact = (Contact*)allocator->alloc(sizeof(Contact));
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

void CollisionManager::createCircleShape(Instance i, float radius, const glm::vec2& position)
{
    if(data.shape[i.index].shape != ShapeType::NONE)
	freeShape(data.shape[i.index].shape, data.shape[i.index].data.ptr);
    
    data.shape[i.index].shape = ShapeType::CIRCLE;
    data.shape[i.index].data.ptr = allocateShape(ShapeType::CIRCLE);
    data.shape[i.index].data.circle->radius = radius;

    AABB aabb;
    aabb.lower = glm::vec2(position.x - radius, position.y - radius);
    aabb.upper = glm::vec2(position.x + radius, position.y + radius);

    data.node[i.index] = tree.createProxy(data.entities[i.index], aabb);

    broadTest(i);	     
}

void CollisionManager::createPolygonShape(Instance i, glm::vec2* vertices, uint32_t count, const glm::vec2& position)
{
    if(data.shape[i.index].shape != ShapeType::NONE)
	freeShape(data.shape[i.index].shape, data.shape[i.index].data.ptr);
    
    data.shape[i.index].shape = ShapeType::POLYGON;
    data.shape[i.index].data.ptr = allocateShape(ShapeType::POLYGON);

    data.shape[i.index].data.polygon->set(vertices, count);

    // TODO (daniel): Can we make a better AABB approx for polygons
    AABB aabb = data.shape[i.index].data.polygon->computeAABB();

    float len = glm::max(aabb.upper.x - aabb.lower.x, aabb.upper.y - aabb.lower.y) / 2;
    
    aabb.lower.x -= len; 
    aabb.upper.x += len;

    aabb.lower.y -= len; 
    aabb.upper.y += len;
    
    aabb.lower += position;
    aabb.upper += position;

    data.node[i.index] = tree.createProxy(data.entities[i.index], aabb);

    broadTest(i);
}

void CollisionManager::createRayShape(Instance i, const glm::vec2& direction, const glm::vec2& position)
{
    if(data.shape[i.index].shape != ShapeType::NONE)
	freeShape(data.shape[i.index].shape, data.shape[i.index].data.ptr);
    
    data.shape[i.index].shape = ShapeType::RAY;
    data.shape[i.index].data.ptr = allocateShape(ShapeType::RAY);

    data.shape[i.index].data.ray->direction = direction;

    glm::vec2 a = position;
    glm::vec2 b = position + direction;

    glm::vec2 min = glm::min(a, b);
    glm::vec2 max = glm::max(a, b);
    
    AABB aabb;
    aabb.lower = min;
    aabb.upper = max;
    
    data.node[i.index] = tree.createProxy(data.entities[i.index], aabb);

    broadTest(i);
}

void* CollisionManager::allocateShape(ShapeType type)
{
    switch(type)
    {
    case ShapeType::CIRCLE:
	return circlePool.alloc(0);
	break;
    case ShapeType::POLYGON:
	return polygonPool.alloc(0);
	break;
    case ShapeType::RAY:
	return rayPool.alloc(0);
	break;
    }
    return nullptr;
}

void CollisionManager::freeShape(ShapeType type, void* ptr)
{
    switch(type)
    {
    case ShapeType::CIRCLE:
	circlePool.free(ptr);
	break;
    case ShapeType::POLYGON:
	polygonPool.free(ptr);
	break;
    case ShapeType::RAY:
	rayPool.free(ptr);
	break;
    }
}

void CollisionManager::setRadius(Instance i, float radius)
{
    assert(data.shape[i.index].shape == ShapeType::CIRCLE);
    data.shape[i.index].data.circle->radius = radius;

    // TODO (daniel): Update the proxy bound for the circle and perform new broad test
}

void CollisionManager::setDirection(Instance i, const glm::vec2& direction, const glm::vec2& position)
{
    assert(data.shape[i.index].shape == ShapeType::RAY);
    data.shape[i.index].data.ray->direction = direction;

    glm::vec2 a = position;
    glm::vec2 b = position + direction;

    glm::vec2 min = glm::min(a, b);
    glm::vec2 max = glm::max(a, b);
    
    AABB aabb;
    aabb.lower = min;
    aabb.upper = max;

    tree.updateProxyBounds(data.node[i.index], aabb);
    
    broadTest(i);
}

