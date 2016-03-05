ParticleEngine::ParticleEngine(dcutil::StackAllocator* _stack, dcfx::Context* _renderCtx, SpriteBatch* _sb)
	:
	stack(_stack),
	renderCtx(_renderCtx),
	sb(_sb),
	emittersInPlayCount(0)
{
    srand((unsigned int)time(NULL));

    reset();
}

void ParticleEngine::reset()
{
    emittersInPlayCount = 0;

    effectHandles = dcutil::HandleAllocator<MaxEffects>();
    emitterHandles = dcutil::HandleAllocator<MaxEmitters>();

    for(int i = 0; i < MaxEffects; ++i)
    {
	effects[i] = ParticleEffect();
	effectsInPlay[i] = nullptr;
    }
    
    for(int i = 0; i < MaxEmitters; ++i)
    {
	emitters[i] = ParticleEmitter();
	emittersInPlay[i] = nullptr;
    }

    memoryPools[0] = dcutil::PoolAllocator(
	stack->alloc(s_particleBufferSize[0] * 10000 * sizeof(Particle)),
	s_particleBufferSize[0] * sizeof(Particle), 10000);
    
    memoryPools[1] = dcutil::PoolAllocator(
	stack->alloc(s_particleBufferSize[1]  * 100 * sizeof(Particle)),
	s_particleBufferSize[1] * sizeof(Particle), 100);
    
    memoryPools[2] = dcutil::PoolAllocator(
	stack->alloc(s_particleBufferSize[2] * 10 * sizeof(Particle)),
	s_particleBufferSize[2] * sizeof(Particle), 10);
}

ParticleEffectHandle ParticleEngine::createHandle(ParticleEffectDescription* desc)
{
    ParticleEffectHandle handle = { effectHandles.alloc() };
    assert(handle.index < ARRAYSIZE(effects));
    ParticleEffect* effect = &effects[handle.index];  
    effect->desc = desc;

    ParticleEmitterDescription* emitterDesc = desc->emitter;
    for(uint32_t i = 0; i < desc->count; ++i)
    {
	EmitterHandle emitterHandle = { emitterHandles.alloc() };
	assert(emitterHandle.index < ARRAYSIZE(emitters));
	ParticleEmitter* emitter = &emitters[emitterHandle.index];
		
	assert(i < ARRAYSIZE(effect->emitters));
	effect->emitters[i] = emitterHandle;

	ParticleBufferSizeTier tier = (ParticleBufferSizeTier)emitterDesc->bufferSizeTier;

	// Empty buffer allocate it
	if(emitter->b0.particles == nullptr)
	{
	    emitter->b0.particles = (Particle*)memoryPools[tier].alloc(0);
	    emitter->b1.particles = (Particle*)memoryPools[tier].alloc(0);
	}
	else
	{
	    // Buffer exist but of wrong tier
	    if(emitter->desc->bufferSizeTier != emitterDesc->bufferSizeTier)
	    {
		ParticleBufferSizeTier oldTier = (ParticleBufferSizeTier)emitter->desc->bufferSizeTier;

		memoryPools[oldTier].free(emitter->b0.particles);
		memoryPools[oldTier].free(emitter->b1.particles);

		emitter->b0.particles = (Particle*)memoryPools[tier].alloc(0);
		emitter->b1.particles = (Particle*)memoryPools[tier].alloc(0);
	    }
	}

	emitter->b0.size = s_particleBufferSize[tier];
	emitter->b0.used = 0;

	emitter->b1.size = s_particleBufferSize[tier];
	emitter->b1.used = 0;

	emitter->active = &emitter->b0;
	emitter->second = &emitter->b1;

	emitter->time = emitterDesc->spawnTime;
	emitter->lifetime = 0;

	// Remove 
        emitter->state = EmitterState::EMITTING;

	emitter->desc = emitterDesc;
	
	emitterDesc = emitterDesc->next;
    }

    return handle;
};

void ParticleEngine::deleteHandle(ParticleEffectHandle handle)
{
    ParticleEffect* effect = &effects[handle.index];
    
    int count = effect->desc->count;
    for(int i = 0; i < count; ++i)
    {
	emitters[effect->emitters[i].index].state = EmitterState::ENDING;
    }
}

void ParticleEngine::play(ParticleEffectHandle handle, uint32_t frames)
{
    effectsInPlay[handle.index] = &effects[handle.index];

    ParticleEffect* effect = effectsInPlay[handle.index];
    
    for(uint32_t j = 0; j < effect->desc->count; ++j)
    {
	ParticleEmitter* emitter = &emitters[effect->emitters[j].index];

	if(emitter->desc->spawnTime == 0)
	{
	    spawn(emitter);
	}	
    }

    float dt = 0.016f;
    for(uint32_t i = 0; i < frames; ++i)
    {
	update(dt);
	swap();
    }
}

void ParticleEngine::stop(ParticleEffectHandle handle)
{
    effectsInPlay[handle.index] = nullptr;
}

void ParticleEngine::setPosition(ParticleEffectHandle handle, const glm::vec2& position)
{
    effects[handle.index].position = position;
}

void ParticleEngine::setRotation(ParticleEffectHandle handle, float rotation)
{
    effects[handle.index].rotation = rotation;
}

void ParticleEngine::setDepth(ParticleEffectHandle handle, float depth)
{
    effects[handle.index].depth = depth;
}
   
void ParticleEngine::update(float dt)
{
    prepass();

    emit(dt);  

    simulate(dt);
}

// The purpose of the prepass is to sort the emitters into thier separate venues
void ParticleEngine::prepass()
{
    TIME_BLOCK(Particle_Prepass);

    emittersInPlayCount = 0;
    
    for(int i = 0; i < ARRAYSIZE(effectsInPlay); ++i)
    {
	ParticleEffect* effect = effectsInPlay[i];
	if(effect)
	{
	    int finnishedEmitters = 0;
	    for(uint32_t j = 0; j < effect->desc->count; ++j)
	    {
		ParticleEmitter* emitter = &emitters[effect->emitters[j].index];

		// This is where emitter sorting is done
		if(emitter->state != EmitterState::FINNISHED)
		{
		    emitter->position = effect->position;
		    emitter->rotation = effect->rotation;
		    emitter->depth = effect->depth;

		    // This needs to be segregated 
		    emittersInPlay[emittersInPlayCount++] = emitter;
		}
		else
		{
		    finnishedEmitters++;
		}    
	    }

	    // If simulation is ended for all emitters
	    if(finnishedEmitters == effect->desc->count)
	    {
		stop({(uint16_t)i});

		for(uint32_t j = 0; j < effect->desc->count; ++j)
		{
		    emitterHandles.free(effect->emitters[j].index);
		}
		
		effectHandles.free(i);
	    }
	}
    }
}

inline float randBetween(float a, float b)
{
    float f = rand() / (float)RAND_MAX;
    return a + (b - a) * f;
}

void ParticleEngine::spawn(ParticleEmitter* emitter)
{
    for(int32_t i = 0; i < emitter->desc->particlesPerEmit; i++)
    {
	Particle p;
	p.position = emitter->desc->position;

	// Compute relative position
	if(emitter->desc->relative)
	{
	    float angle = atan2(p.position.y, p.position.x);

	    glm::vec2 npos;
	    npos.x = cos(-emitter->rotation + angle);
	    npos.y = sin(-emitter->rotation + angle);

	    npos *= glm::length(p.position);
		    
	    p.position = emitter->position + npos;
	}

	glm::vec2 force = emitter->desc->force;
	float forceAngle = atan2(force.y, force.x);
		
	float spreadAngle = glm::radians(emitter->desc->spread);

	// Random between 0 and spread angle
	float randomAngle = (rand() / (float)RAND_MAX) * spreadAngle;

	// Compute relative rotation
	if(emitter->desc->relative)
	{
	    force.x = cos(randomAngle + forceAngle - emitter->rotation);
	    force.y = sin(randomAngle + forceAngle - emitter->rotation);
	}
	else
	{
	    force.x = cos(randomAngle + forceAngle);
	    force.y = sin(randomAngle + forceAngle);
	}	

	force *= glm::length(emitter->desc->force);

        // NOTE (daniel): Particles assumed to have the same mass for now
	p.acceleration = force;	
	p.velocity = glm::vec2(0,0);
	
	p.color.r = randBetween(emitter->desc->colorMin.r, emitter->desc->colorMax.r);
	p.color.g = randBetween(emitter->desc->colorMin.g, emitter->desc->colorMax.g);
	p.color.b = randBetween(emitter->desc->colorMin.b, emitter->desc->colorMax.b);
	p.color.a = randBetween(emitter->desc->colorMin.a, emitter->desc->colorMax.a);

	// NOTE (daniel): Particles assumed to be rectangular so only uniform scaling
	float size = randBetween(emitter->desc->sizeMin, emitter->desc->sizeMax);
	p.size = glm::vec2(size,size);
	p.rotation = glm::radians(randBetween(emitter->desc->rotationMin, emitter->desc->rotationMax));
	p.depth = randBetween(emitter->desc->depthMin, emitter->desc->depthMax);
	
	p.lifetime = randBetween(emitter->desc->lifetimeMin, emitter->desc->lifetimeMax);
	p.texture = emitter->desc->texture;
	p.time = 0;

	assert(emitter->active->used < emitter->active->size && "Particle Buffer Overflow!");
	    
	emitter->active->particles[emitter->active->used] = p;
	emitter->active->used++;
    }
}

void ParticleEngine::emit(float dt)
{
    TIME_BLOCK(Particle_Emit);

    // TODO (daniel): Itterate _only_ on those emitters that _can_ spawn and _should_ spawn.
    for(uint32_t i = 0; i < emittersInPlayCount; ++i)
    {
	// TODO (daniel): Remove this state check
	ParticleEmitter* emitter = emittersInPlay[i];
	if(emitter->state == EmitterState::ENDING)
	    continue;
	
	emitter->time += dt;
	emitter->lifetime += dt;

	// Emitters lifetime is over (Transition from EMITTING to ENDING)
	if(emitter->desc->lifetime != 0 && emitter->lifetime >= emitter->desc->lifetime)
	{
	    emitter->state = EmitterState::ENDING;
	    continue;
	}

	// Should we spawn? If so then spawn
	if(emitter->desc->spawnTime != 0 && emitter->time >= emitter->desc->spawnTime)
	{
	    spawn(emitter);
	    
	    emitter->time = 0;
	}
    }

    // Transition from ENDING to FINNISHED
    for(uint32_t i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];
	if(emitter->state == EmitterState::ENDING && emitter->active->used == 0)
	{
	    emitter->state = EmitterState::FINNISHED;
	}
    }
}

void ParticleEngine::simulate(float dt)
{
    TIME_BLOCK(Particle_Simulate);
    
    for(uint32_t i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];

	for (uint32_t j = 0; j < emitter->active->used; ++j)
	{
	    Particle* p = &emitter->active->particles[j];

	    p->velocity += p->acceleration * dt;
	    
	    p->position += p->velocity * dt;

	    p->time += dt;

	    if(p->lifetime == 0 || p->time <= p->lifetime)
	    {
		emitter->second->particles[emitter->second->used++] = *p;
	    }
	}	
    }
}

void ParticleEngine::render(int view)
{
    TIME_BLOCK(Particle_Render);
    
    for(uint32_t i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];
	
	for (uint32_t j = 0; j < emitter->second->used; ++j)
	{
	    Particle* p = &emitter->second->particles[j];

	    glm::vec4 color = p->color;

	    // NOTE (daniel): This is an argument for not using lifetime == 0 as infinite
	    if(p->lifetime != 0)
		color += (emitter->desc->endColor - p->color) * (p->time / p->lifetime);

	    float depth = emitter->depth + p->depth;
	    
	    glm::vec2 projectedXY = (1.0f / depth) * p->position;
	    float scale = 1.0f / depth;

	    depth = glm::round(depth);
	    
	    sb->setTexture(p->texture->handle);
	    sb->setDestination(glm::vec4(projectedXY.x,
					 projectedXY.y,
					 emitter->desc->texture->width * p->size.x * scale,
					 emitter->desc->texture->height * p->size.y * scale));
	    sb->setSource(glm::vec4(0,0,1,1));
	    sb->setRotation(p->rotation);
	    sb->setOrigin(glm::vec2(0.5, 0.5));
	    sb->setColor(color);
	    sb->setDepth(depth);
	    sb->submit();
	}
    }
    
    swap();
}

void ParticleEngine::debug(int view)
{
    for(uint32_t i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];

	char out[100];
	sprintf(out, "Emitter:%d Used:%d Size:%d\n", i, emitter->second->used, emitter->second->size);
	
        sb->write(out, glm::vec2(10, 50 + i * 20));
    }
}

void ParticleEngine::swap()
{
    for(uint32_t i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];

	ParticleBuffer* temp = emitter->active;

	emitter->active = emitter->second;

	emitter->second = temp;
	emitter->second->used = 0;
    }
}
