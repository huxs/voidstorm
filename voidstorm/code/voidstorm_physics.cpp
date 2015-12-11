void ContactManager::add(dcutil::StackAllocator* stack, uint32_t key, uint32_t reference)
{
     int hash = dcutil::sdbm32((char*)&key, sizeof(key));
     int slot = (ARRAYSIZE(map)-1) & hash;

     ContactNode* node = map + slot;
     do
     {
	 if(node->reference == reference)
	 {
	     node->key = key;
	     break;
	 }
	 
	 if(node->reference == 0 && !node->next)
	 {
	     node->next = (ContactNode*)stack->alloc(sizeof(ContactNode));
	     node = node->next;
	     node->key = 0;
	     node->reference = 0;
	     node->next = nullptr;
	 }

	 if(node->reference == 0)
	 {
	     node->key = key;
	     node->reference = reference;
	     break;
	 }

	 node = node->next;
	 
     }  while(node);
}

uint32_t ContactManager::lookup(uint32_t key)
{
    int hash = dcutil::sdbm32((char*)&key, sizeof(key));
    int slot = (ARRAYSIZE(map)-1) & hash;

    uint32_t reference = 0;
    ContactNode* node = map + slot;
    do
    {
	if(node == nullptr)
	    break;

	if(node->key == key)
	{
	    reference = node->reference;
	}

	node = node->next;
	   
    } while(node);

    return reference;
}

void PhysicsSimulator::simulate(World* world, float dt, LineRenderer* linerenderer)
{
    TIME_BLOCK(Physics_Simulate);

    numManifolds = 0;
    ContactManager cm;

    for(uint32_t i = 1; i < world->physics.data.used; ++i)
    {
	glm::vec2 friction = world->physics.data.velocity[i] * 1.2f;
	glm::vec2 totalForce = world->physics.data.force[i] - friction;
			
	world->physics.data.acceleration[i] = (totalForce / world->physics.data.mass[i]);
	world->physics.data.velocity[i] += world->physics.data.acceleration[i] * dt;
    }

    for(uint32_t i = 1; i < world->collisions.data.used; ++i)
    {
	Entity e = world->collisions.data.entities[i];
	uint32_t type = world->collisions.data.type[i];
	uint32_t mask = world->collisions.data.mask[i];
	CollisionManager::ShapeData shapeData = world->collisions.data.shape[i];

	TransformManager::Instance transform = world->transforms.lookup(e);
	glm::vec2 pos = world->transforms.data.position[transform.index];
	glm::vec2 newPosition = pos;

	PhysicsManager::Instance physics = world->physics.lookup(e);	
	glm::vec2 vel = world->physics.data.velocity[physics.index];
	glm::vec2 deltaVel = vel * dt;

	bool hit = false;
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

		uint32_t a = glm::min(transform.index, otherTransform.index);
		uint32_t b = glm::max(transform.index, otherTransform.index);
		    
		// https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
		uint32_t key = (a + b) * (a + b + 1) / 2 + a;			

		uint32_t reference = cm.lookup(key);
		if(reference == 0)
		{		    
		    cm.add(g_permStackAllocator, key, a);
		    
		    if (c->callback == NULL)
		    {
			PRINT("ERROR: Entity %d colliding with %d has null callback.\n", e.index(), other_e.index());
			break;
		    }

		    PhysicsManager::Instance otherPhysics = world->physics.lookup(other_e);	
		    glm::vec2 otherVel = world->physics.data.velocity[otherPhysics.index];
		    glm::vec2 otherDeltaVel = vel * dt;
		    
		    // Execute collision routine
		    Manifold manifold = c->callback(world, deltaVel, otherDeltaVel, transform, shapeData, otherTransform, otherShapeData);
		    if(manifold.numContacts > 0)
		    {
			// Callback
			CollisionResponderManager::Instance responder = world->responders.lookup(e);
			if(responder.index != 0)
			{
			    world->responders.addEntity(responder, other_e, manifold.contacts[0].position, manifold.contacts[0].normal);
			}

			CollisionResponderManager::Instance otherResponder = world->responders.lookup(other_e);
			if(otherResponder.index != 0)
			{
			    world->responders.addEntity(responder, e, manifold.contacts[0].position, manifold.contacts[0].normal);   
			}

			// If both objects have physics component add the manifold for impulse resolution
			if(physics.index != 0 && otherPhysics.index != 0)
			{
			    hit = true;
			    assert(numManifolds < ARRAYSIZE(manifolds));
			    manifold.pinstA = physics;
			    manifold.pinstB = otherPhysics;		    
			    manifolds[numManifolds++] = manifold;
			}
		    }		    	        			
		}
	    }
		
	    c = c->next;
	}
	    
	// If we never collided with anything move the entity
	if(!hit)
	{
	    newPosition += deltaVel;
	    
	    world->transforms.data.position[transform.index] = newPosition;
	}		

	// If the entity moved perform new broad test
	glm::vec2 displacement = newPosition - pos;
	float dispLength = glm::length(displacement);
	if(dispLength > 0)
	{
	    CollisionManager::Instance instance = { i };
	    DbvtNode* node = world->collisions.data.node[i];
	    
	    world->collisions.resetContacts(instance);
	    world->collisions.addContacts(instance);	    
	    world->collisions.tree.moveProxy(node, displacement);
	}	
    }
}

// Resolve collisions
void PhysicsSimulator::foo(World* world)
{
    for(uint32_t i = 0; i < numManifolds; ++i)
    {
	TransformManager::Instance transformA = manifolds[i].instA;
	TransformManager::Instance transformB = manifolds[i].instB;
	
	PhysicsManager::Instance physicsA = manifolds[i].pinstA;
	PhysicsManager::Instance physicsB = manifolds[i].pinstB;
	
	glm::vec2 velA = world->physics.data.velocity[physicsA.index];
	glm::vec2 velB = world->physics.data.velocity[physicsB.index];
	
	glm::vec2 collisionNormal = manifolds[i].contacts[0].normal;
	
	glm::vec2 relVel = velB - velA;
	float contactVel = glm::dot(relVel, collisionNormal);

	if(contactVel > 0)
	{
	    float invMassA = 1.0f / world->physics.data.mass[physicsA.index];
	    float invMassB = 1.0f / world->physics.data.mass[physicsB.index];
	    
	    // restitution
	    float e = 1.0f; // miniumim of both materials

	    // impulse
	    float j = -(1.0f + e) * contactVel;
	    j /= (invMassA + invMassB);

	    glm::vec2 impulse = j * collisionNormal;

	    world->physics.data.velocity[physicsA.index] -= invMassA * impulse;
	    world->physics.data.velocity[physicsB.index] += invMassB * impulse;	    
	}
    }
}
