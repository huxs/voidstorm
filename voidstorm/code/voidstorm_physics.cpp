

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

const float PhysicsSimulator::PhysicsHz = 1.0f / 60.0f;

PhysicsSimulator::PhysicsSimulator()
	    :
	    numManifolds(0),
	    accumulator(0)
	{
	    previousState.position = (glm::vec2*)g_permStackAllocator->alloc(VOIDSTORM_TRANSFORM_COMPONENT_COUNT * sizeof(glm::vec2));
	    interpolatedState.position = (glm::vec2*)g_permStackAllocator->alloc(VOIDSTORM_TRANSFORM_COMPONENT_COUNT * sizeof(glm::vec2));
	}

void PhysicsSimulator::update(World* world, float dt, LineRenderer* linerenderer)
{
    accumulator += dt;

    while(accumulator >= PhysicsHz)
    {
	// Store the previous state
	memcpy(previousState.position, world->transforms.data.position, world->transforms.data.used * sizeof(glm::vec2));	

	integrateVelocity(world, PhysicsHz);
	narrowCollision(world, PhysicsHz, linerenderer);
	resolveCollisions(world);
	updateVelocity(world, PhysicsHz);
	
	accumulator -= PhysicsHz;
    }

    /* Since physics simulation is decoupled from the game loop the renderer can display an incorrect state.
       We solve that by linear interpolating between the previous state and the current state.
       http://gafferongames.com/game-physics/fix-your-timestep/
    */
    float alpha = accumulator / PhysicsHz;

    for(uint32_t i = 1; i < world->transforms.data.used; ++i)
    {
	glm::vec2 pos = world->transforms.data.position[i];
	glm::vec2 prevPos = previousState.position[i];

        interpolatedState.position[i] = pos * alpha + prevPos * (1.0f - alpha);
    }	    
}

void PhysicsSimulator::integrateVelocity(World* world, float dt)
{
    TIME_BLOCK(Physics_IntegrateVelocity);

    for(uint32_t i = 1; i < world->physics.data.used; ++i)
    {
	// NOTE (daniel): Hard coded friction constant
	glm::vec2 friction = world->physics.data.velocity[i] * 1.2f;
	glm::vec2 totalForce = world->physics.data.force[i] - friction;
			
	world->physics.data.acceleration[i] = (totalForce / world->physics.data.mass[i]);
	world->physics.data.velocity[i] += world->physics.data.acceleration[i] * dt;
    }
}

void PhysicsSimulator::narrowCollision(World* world, float dt, LineRenderer* linerenderer)
{
    TIME_BLOCK(Physics_NarrowCollision);

    numManifolds = 0;
    ContactManager cm;

    for(uint32_t i = 1; i < world->collisions.data.used; ++i)
    {
	Entity e = world->collisions.data.entities[i];
	uint32_t type = world->collisions.data.type[i];
	uint32_t mask = world->collisions.data.mask[i];
	CollisionManager::ShapeData shapeData = world->collisions.data.shape[i];

	TransformManager::Instance transform = world->transforms.lookup(e);
	glm::vec2 pos = world->transforms.data.position[transform.index];
	glm::vec2 newPosition = pos;

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
		    
		    // Execute collision routine
		    Manifold manifold = c->callback(world, transform, shapeData, otherTransform, otherShapeData, linerenderer);
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
			    world->responders.addEntity(otherResponder, e, manifold.contacts[0].position, manifold.contacts[0].normal);   
			}

			PhysicsManager::Instance physics = world->physics.lookup(e);
			PhysicsManager::Instance otherPhysics = world->physics.lookup(other_e);
			
			// If both objects have physics component add the manifold for impulse resolution
			if(physics.index != 0 && otherPhysics.index != 0)
			{
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
    }
}

// Impulse resolution
void PhysicsSimulator::resolveCollisions(World* world)
{
    TIME_BLOCK(Physics_ResolveCollisions);

    for(uint32_t it = 0; it < 20; ++it)
    {   
	for(uint32_t i = 0; i < numManifolds; ++i)
	{
	    TransformManager::Instance transformA = manifolds[i].instA;
	    TransformManager::Instance transformB = manifolds[i].instB;
	
	    PhysicsManager::Instance physicsA = manifolds[i].pinstA;
	    PhysicsManager::Instance physicsB = manifolds[i].pinstB;
	
	    glm::vec2 velA = world->physics.data.velocity[physicsA.index];
	    glm::vec2 velB = world->physics.data.velocity[physicsB.index];

	    int flagA = world->physics.data.flags[physicsA.index];
	    int flagB = world->physics.data.flags[physicsB.index];
	
	    glm::vec2 collisionNormal = manifolds[i].contacts[0].normal;
	
	    glm::vec2 relVel = velB - velA;
	    float contactVel = glm::dot(relVel, collisionNormal);

	    // Only perform impulse resolution if the objects are moving into eachother
	    if(contactVel > 0)
	    {
		float invMassA = 1.0f / world->physics.data.mass[physicsA.index];
		float invMassB = 1.0f / world->physics.data.mass[physicsB.index];

		float restitutionA = world->physics.data.restitution[physicsA.index];
		float restitutionB = world->physics.data.restitution[physicsB.index];
	    
		// Restitution
		// TODO (daniel): Use the minimum instead
		float e = glm::max(restitutionA, restitutionB);

		// Impulse
		float j = -(1.0f + e) * contactVel;
		j /= (invMassA + invMassB);

		glm::vec2 impulse = j * collisionNormal;

		// TODO (daniel): Rotational velocity

		// TODO (daniel): Friction between objects
		
		// TODO (daniel): Cleanup the flags
		if(flagA == 0)
		    world->physics.data.velocity[physicsA.index] -= invMassA * impulse;

		if(flagB == 0)
		    world->physics.data.velocity[physicsB.index] += invMassB * impulse;	    
	    }

	    // TODO (daniel): Penetration Resolution
	    // Due to floating point issues, objects can penetrate eachother slightly
	}
    }
}

void PhysicsSimulator::updateVelocity(World* world, float dt)
{
    TIME_BLOCK(Physics_UpdateVelocity);
    
    for(uint32_t i = 1; i < world->physics.data.used; ++i)
    {
	Entity e = world->physics.data.entities[i];
	glm::vec2 vel = world->physics.data.velocity[i];
	glm::vec2 deltaVel = vel * dt;

	TransformManager::Instance transform = world->transforms.lookup(e);
	glm::vec2 pos = world->transforms.data.position[transform.index];

	glm::vec2 newPosition = pos + deltaVel;
	
	glm::vec2 displacement = newPosition - pos;
	float dispLength = glm::length(displacement);
	if(dispLength > 0)
	{
	    CollisionManager::Instance collision = world->collisions.lookup(e);
	    DbvtNode* node = world->collisions.data.node[collision.index];
	    
	    world->collisions.broadTest(collision);	    
	    world->collisions.tree.moveProxy(node, displacement);

	    world->transforms.data.position[transform.index] = newPosition;
	}
    }
}
