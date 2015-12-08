static void simulate(World* world, float dt)
{
    TIME_BLOCK(Physics_Simulate);
    
    for(uint32_t i = 1; i < world->physics.data.used; ++i)
    {
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

	// Itterations
	for(int k = 0; k < 1; ++k)
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

		    // Execute collision routine
		    result = c->callback(world, deltaVel, transform, shapeData, otherTransform, otherShapeData);
		    
		    if(result.hit)
		    {
			glm::vec2 desiredPos = newPosition + deltaVel;
		
			newPosition = result.position;

			world->transforms.data.position[transform.index] = newPosition;
		    
			vel = vel - 2 * glm::dot(vel, result.normal) * result.normal;
		    
			world->physics.data.velocity[inst.index] = vel;
        
			deltaVel = desiredPos - newPosition;
			deltaVel = deltaVel - 2 * glm::dot(deltaVel, result.normal) *  result.normal;
			
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
				    world->responders.data.collidedWith[responder.index].position[entityCount] = result.position;
				    world->responders.data.collidedWith[responder.index].normal[entityCount++] = result.normal;
				}
			    }
			}
		    }
		}
		
		c = c->next;
	    }


	    // If we never collided with anything move the entity
	    if(!result.hit)
	    {
		newPosition += deltaVel;
		
		break;
	    }		
	}

	// If the entity moved perform new contact test
	glm::vec2 displacement = newPosition - pos;
	float dispLength = glm::length(displacement);
	if(dispLength > 0)
	{
	    CollisionManager::Instance instance;
	    instance.index = i;

	    world->collisions.resetContacts(instance);
	    world->collisions.addContacts(instance);
	    
	    world->collisions.tree.moveProxy(node, displacement);

	    world->transforms.data.position[transform.index] = newPosition;
	}
    }   
}
