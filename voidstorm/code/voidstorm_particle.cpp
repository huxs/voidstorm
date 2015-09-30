ParticleEngine::ParticleEngine(dcutil::Stack* _stack, dcfx::Context* _renderCtx, SpriteBatch* _sb)
	:  stack(_stack), renderCtx(_renderCtx), sb(_sb), emittersInPlayCount(0)
{
    srand((unsigned int)time(NULL));
}

ParticleHandle ParticleEngine::createHandle(ParticleEffectDescription* desc)
{
    ParticleHandle handle = { effectHandles.alloc() };

    ParticleEffect* effect = &effects[handle.index];  
    effect->desc = desc;

    ParticleEmitterDescription* emitterDesc = desc->emitter;
    for(int i = 0; i < desc->count; ++i)
    {
	EmitterHandle emitterHandle = { emitterHandles.alloc() };

	effect->emitters[i] = emitterHandle;

	ParticleEmitter* emitter = &emitters[emitterHandle.index];
	emitter->emitter = emitterDesc;

	static const int NumParticles = 100;

	if(emitter->b0.particles == nullptr)
	{
	    emitter->b0.particles = (Particle*)stack->alloc(NumParticles * sizeof(Particle));
	    emitter->b1.particles = (Particle*)stack->alloc(NumParticles * sizeof(Particle));
	}

	emitter->b0.size = NumParticles;
	emitter->b0.used = 0;

	emitter->b1.size = NumParticles;
	emitter->b1.used = 0;

	emitter->active = &emitter->b0;
	emitter->second = &emitter->b1;

	emitter->time = emitter->emitter->spawnTime;
	emitter->lifetime = 0;
        emitter->state = EmitterState::EMITTING;

	emitterDesc = emitterDesc->next;
    }

    return handle;
};


void ParticleEngine::deleteHandle(ParticleHandle handle)
{
    ParticleEffect* effect = &effects[handle.index];
    
    int count = effect->desc->count;
    for(int i = 0; i < count; ++i)
    {
	emitters[effect->emitters[i].index].state = EmitterState::ENDING;
    }
}

void ParticleEngine::play(ParticleHandle handle)
{
    effectsInPlay[handle.index] = &effects[handle.index];
}

void ParticleEngine::stop(ParticleHandle handle)
{
    effectsInPlay[handle.index] = nullptr;
}

void ParticleEngine::setPosition(ParticleHandle handle, const glm::vec2& position)
{
    effects[handle.index].position = position;
}

void ParticleEngine::setRotation(ParticleHandle handle, float rotation)
{
    effects[handle.index].rotation = rotation;
}

void ParticleEngine::setDepth(ParticleHandle handle, float depth)
{
    effects[handle.index].depth = depth;
}
   
void ParticleEngine::update(float dt)
{
    prepass();

    emit(dt);  

    simulate(dt);
}

inline float randBetween(float a, float b)
{
    float f = rand() / (float)RAND_MAX;
    return a + (b - a) * f;
}

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
	    for(int j = 0; j < effect->desc->count; ++j)
	    {
		ParticleEmitter* emitter = &emitters[effect->emitters[j].index];

		if(emitter->state != EmitterState::FINNISHED)
		{
		    emitter->position = effect->position;
		    emitter->rotation = effect->rotation;
		    emitter->depth = effect->depth;

		    emittersInPlay[emittersInPlayCount++] = emitter;
		}
		else
		{
		    finnishedEmitters++;
		}    
	    }

	    if(finnishedEmitters == effect->desc->count)
	    {
		stop({(uint16_t)i});

		for(int j = 0; j < effect->desc->count; ++j)
		{
		    emitterHandles.free(effect->emitters[j].index);
		}
		
		effectHandles.free(i);
	    }
	}
    }
}

void ParticleEngine::emit(float dt)
{
    TIME_BLOCK(Particle_Emit);
    
    for(int i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];
	if(emitter->state == EmitterState::ENDING)
	    continue;
	
	emitter->time += dt;
	emitter->lifetime += dt;

	if(emitter->emitter->lifetime != 0 && emitter->lifetime >= emitter->emitter->lifetime)
	{
	    emitter->state = EmitterState::ENDING;
	    continue;
	}
	
	if(emitter->time >= emitter->emitter->spawnTime)
	{
	    for(int j = 0; j < emitter->emitter->particlesPerEmit; j++)
	    {
		Particle p;
		p.position = emitter->emitter->position;
		
		if(emitter->emitter->relative)
		{
		    float angle = atan2(p.position.y, p.position.x);

		    glm::vec2 npos;
		    npos.x = cos(-emitter->rotation + angle);
		    npos.y = sin(-emitter->rotation + angle);

		    npos *= glm::length(p.position);
		    
		    p.position = emitter->position + npos;

		}

		glm::vec2 f = emitter->emitter->force;
		float dangle = atan2(f.y, f.x);
		
		float angle = glm::radians(emitter->emitter->spread);

		float rangle = (rand() / (float)RAND_MAX) * angle;

		if(emitter->emitter->relative)
		{
		    f.x = cos(rangle + dangle -emitter->rotation);
		    f.y = sin(rangle + dangle -emitter->rotation);
		}
		else
		{
		    f.x = cos(rangle + dangle);
		    f.y = sin(rangle + dangle);
		}
	

		f *= glm::length(emitter->emitter->force);
	    
		p.acceleration = f;
		p.velocity = glm::vec2(0,0);
		p.color = emitter->emitter->startColor;

		float size = randBetween(emitter->emitter->sizeMin, emitter->emitter->sizeMax);
		p.size = glm::vec2(size,size);

		p.rotation = glm::radians(randBetween(emitter->emitter->rotationMin, emitter->emitter->rotationMax));
		
		p.lifetime = randBetween(emitter->emitter->lifetimeMin, emitter->emitter->lifetimeMax);
		p.time = 0;
		p.texture = emitter->emitter->texture;

		assert(emitter->active->used < emitter->active->size);
	    
		emitter->active->particles[emitter->active->used] = p;
		emitter->active->used++;
	    }
	    
	    emitter->time = 0;
	}
    }

    for(int i = 0; i < emittersInPlayCount; ++i)
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
    
    for(int i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];

	for (int j = 0; j < emitter->active->used; ++j)
	{
	    Particle* p = &emitter->active->particles[j];

	    p->velocity += p->acceleration * dt;
	    
	    p->position += p->velocity * dt;

	    p->time += dt;

	    if(p->time <= p->lifetime)
	    {
		emitter->second->particles[emitter->second->used++] = *p;
	    }
	}	
    }
}

void ParticleEngine::render(int view)
{
    TIME_BLOCK(Particle_Render);
    
    for(int i = 0; i < emittersInPlayCount; ++i)
    {
	ParticleEmitter* emitter = emittersInPlay[i];
	
	for (int j = 0; j < emitter->second->used; ++j)
	{
	    Particle* p = &emitter->second->particles[j];
	    
	    glm::vec4 c = p->color + (emitter->emitter->endColor - p->color) * (p->time / p->lifetime);

	    glm::vec2 projectedXY = (1.0f / emitter->depth) * p->position;
	    float scale = 1.0f / emitter->depth;
	    
	    sb->setTexture(p->texture->handle);
	    sb->setDestination(glm::vec4(projectedXY.x,
					 projectedXY.y,
					 emitter->emitter->texture->width * p->size.x * scale,
					 emitter->emitter->texture->height * p->size.y * scale));
	    sb->setSource(glm::vec4(0,0,1,1));
	    sb->setRotation(p->rotation);
	    sb->setOrigin(glm::vec2(0.5, 0.5));
	    sb->setColor(c);
	    sb->setDepth(0.0f);
	    sb->submit();
	}

	sb->draw(view);
	
	ParticleBuffer* temp = emitter->active;

	emitter->active = emitter->second;

	emitter->second = temp;
	emitter->second->used = 0;
    }
}
