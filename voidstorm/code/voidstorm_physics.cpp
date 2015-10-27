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

PhysicsWorld::PhysicsWorld(dcutil::StackAllocator* stack, LineRenderer* _linerenderer)
	:
	tree(stack->alloc(sizeof(DbvtNode) * DBVT_NODE_POOL_SIZE)),
	linerenderer(_linerenderer)
{}
    
void PhysicsWorld::simulate(World* world, float dt)
{
    TIME_BLOCK(Physics_Simulate);
    
    for(uint32_t i = 1; i < world->physics.data.used; ++i)
    {
	// TODO: Use ODE!
	glm::vec2 friction = -world->physics.data.velocity[i] * 1.2f;
	glm::vec2 totalForce = world->physics.data.force[i] + friction;
			
	world->physics.data.acceleration[i] = (totalForce / world->physics.data.mass[i]);

	world->physics.data.velocity[i] += world->physics.data.acceleration[i] * dt;
    }
    
    for(uint32_t i = 1; i < world->collisions.data.used; ++i)
    {
	Entity e = world->collisions.data.entities[i];
	uint32_t type = world->collisions.data.type[i];
	uint32_t mask = world->collisions.data.mask[i];
	CollisionManager::ShapeData shapeData = world->collisions.data.shape[i];
	DbvtNode* node = world->collisions.data.node[i];
	
	TransformManager::Instance transform = world->transforms.lookup(e);
	glm::vec2 pos = world->transforms.data.position[transform.index];
	glm::vec2 newPosition = pos;

	PhysicsManager::Instance inst = world->physics.lookup(e);
	
	glm::vec2 vel = world->physics.data.velocity[inst.index];
	glm::vec2 deltaVel = vel * dt;

	ContactResult result;

	// Itterations.
	for(int k = 0; k < 3; ++k)
	{
	    Contact* c = &world->collisions.data.contact[i];	    
	    while(c)
	    {
		Entity other_e = c->entity;

		CollisionManager::Instance otherCollision = world->collisions.lookup(other_e);
		
		uint32_t otherType = world->collisions.data.type[otherCollision.index];
		if(mask & otherType)
		{	
		    CollisionManager::ShapeData otherShapeData = world->collisions.data.shape[otherCollision.index];		    
		    TransformManager::Instance otherTransform = world->transforms.lookup(other_e);

		    result = c->callback(world, deltaVel, transform, shapeData, otherTransform, otherShapeData);
		    
		    if(result.hit)
		    {
			CollisionResponderManager::Instance responder = world->responders.lookup(e);
			if(responder.index != 0)
			{
			    bool addEntity = true;
			    int& entityCount = world->responders.data.collidedWith[responder.index].entityCount;
			    for(int l = 0; l < entityCount; ++l)
			    {
				if(world->responders.data.collidedWith[responder.index].entity[l] == other_e)
				{
				    addEntity = false;
				    break;
				}
			    }
				    
			    if(addEntity)
			    {
				if(entityCount < ARRAYSIZE(world->responders.data.collidedWith[responder.index].entity))
				{
				    world->responders.data.collidedWith[responder.index].entity[entityCount] = other_e;
				    world->responders.data.collidedWith[responder.index].position[entityCount++] = result.position;
				}
			    }
			}
		    }
		}

		if(result.hit)
		{
		    glm::vec2 desiredPos = newPosition + deltaVel;
		
		    newPosition = result.position;

		    world->transforms.data.position[transform.index] = newPosition;
		    
		    vel = vel - glm::dot(vel, result.normal) * result.normal;
		    
		    world->physics.data.velocity[inst.index] = vel;
        
		    deltaVel = desiredPos - newPosition;
		    deltaVel = deltaVel - glm::dot(deltaVel, result.normal) *  result.normal;
		}

		c = c->next;
	    }

	  
	    if(!result.hit)
	    {
		newPosition += deltaVel;
		
		break;
	    }		
	}

	glm::vec2 displacement = newPosition - pos;
	float dispLength = glm::length(displacement);
	if(dispLength > 0)
	{
	    CollisionManager::Instance instance;
	    instance.index = i;

	    world->collisions.resetContacts(instance);
	    
	    Contact contacts[1024];
	    int contactsFound = tree.query(node, contacts);

	    world->collisions.addContacts(instance, contacts, contactsFound);
	    
	    tree.moveProxy(node, displacement);

	    world->transforms.data.position[transform.index] = newPosition;
	}
    }   
}
