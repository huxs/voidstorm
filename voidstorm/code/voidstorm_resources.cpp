Texture* TextureManager::load(const char* name)
{
    Texture* result = textures.lookup(name);
    if(!result)
    {
	char path[150];
	strcpy(path, VOIDSTORM_TEXTURE_DIRECTORY);
	strcat(path, name);
	
	SDL_RWops* handle = SDL_RWFromFile(path, "rb");
	if (handle != nullptr)
	{	    
	    // Get the size of the file in bytes.
	    SDL_RWseek(handle, 0, SEEK_END);
	    size_t size = (size_t)SDL_RWtell(handle);
	    SDL_RWseek(handle, 0, SEEK_SET);

	    char* ptr = (char*)renderCtx->frameAlloc(size);
	    
	    size_t read = SDL_RWread(handle, ptr, size, 1);
	    if (read == 1)
	    {
		dcfx::ImageInfo info;
		dcfx::TextureHandle handle = renderCtx->createTexture(ptr, size, info);
		if(handle.index != dcfx::InvalidHandle)
		{
		    result = textures.add(name);
		    
		    if(result != nullptr)
		    {
			result->handle = handle;
			result->width = info.m_width;
			result->height = info.m_height;
		    }
		    else
		    {
			PRINT("Error adding texture %s. \n", path);
			renderCtx->deleteTexture(handle);
		    }
		}
		else
		{
		    PRINT("Error creating texture %s. \n", path);
		}	    
	    }
	    else
	    {
		PRINT("Error reading texture %s.\n", path);
	    }
	}
	else
	{
	    PRINT("Failed to open texture %s. \n", path);
   	}	
    }    
	
    return result;
}

void TextureManager::remove(Texture* texture)
{
    for(int i = 0; i < ARRAYSIZE(textures.map); ++i)
    {
	ResourceNode<Texture>* node = textures.map + i;
	if(node->resource == texture)
	{
	    renderCtx->deleteTexture(node->resource->handle);

	    texture->handle = { dcfx::InvalidHandle };
	    texture->width = 0;
	    texture->height = 0;
	    
	    node->resource = nullptr;
	    node->hash = -1;
	    break;
	}
    }
}

void TextureManager::clean()
{
    for(int i = 0; i < ARRAYSIZE(textures.map); ++i)
    {
	ResourceNode<Texture>* node = textures.map + i;
	if(node->hash != -1)
	{
	    renderCtx->deleteTexture(node->resource->handle);

	    node->resource->handle = { dcfx::InvalidHandle };
	    node->resource->width = 0;
	    node->resource->height = 0;
	    
	    node->resource = nullptr;
	    node->hash = -1;
	}
    }
}

static void storeEffectParams(ParticleEffectDescription* effect, lua_State* luaState, dcutil::Stack* arena)
{
    lua_getfield(luaState, 1, "emitters");

    int len = (int)lua_objlen(luaState, -1);

    effect->count = len;

    if(effect->emitter == nullptr)
    {
        effect->emitter = (ParticleEmitterDescription*)arena->alloc(sizeof(ParticleEmitterDescription));
    }
    
    ParticleEmitterDescription* emitter = effect->emitter;

    for(int i = 1; i < len + 1; ++i)
    {
	lua_rawgeti(luaState, -1, i);
	
	lua_getfield(luaState, -1, "particlesPerEmit");
        emitter->particlesPerEmit = (int)lua_tonumber(luaState, -1);
	//PRINT("ParticlePerEmit %d\n", emitter->particlesPerEmit);
	lua_pop(luaState, 1);
	
	lua_getfield(luaState, -1, "spawnTime");
        emitter->spawnTime = (float)lua_tonumber(luaState, -1);
	//PRINT("SpawnTime %f\n", emitter->spawnTime);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "lifetime");
        emitter->lifetime = (float)lua_tonumber(luaState, -1);
	//PRINT("LifetimeMin %f\n", emitter->lifetimeMin);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "lifetimeMin");
        emitter->lifetimeMin = (float)lua_tonumber(luaState, -1);
	//PRINT("LifetimeMin %f\n", emitter->lifetimeMin);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "lifetimeMax");
        emitter->lifetimeMax = (float)lua_tonumber(luaState, -1);
	//PRINT("LifetimeMax %f\n", emitter->lifetimeMax);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "sizeMin");
        emitter->sizeMin = (float)lua_tonumber(luaState, -1);
	//PRINT("SizeMin %f\n", emitter->sizeMin);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "sizeMax");
        emitter->sizeMax = (float)lua_tonumber(luaState, -1);
	//PRINT("SizeMax %f\n", emitter->sizeMax);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "rotationMin");
        emitter->rotationMin = (float)lua_tonumber(luaState, -1);
	//PRINT("RotationMin %f\n", emitter->rotationMin);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "rotationMax");
        emitter->rotationMax = (float)lua_tonumber(luaState, -1);
	//PRINT("RotationMax %f\n", emitter->rotationMax);
	lua_pop(luaState, 1);
	
	lua_getfield(luaState, -1, "startColor");
	emitter->startColor = *(glm::vec4*)lua_touserdata(luaState, -1);
	//PRINT("StartColor %f %f %f %f\n", emitter->startColor.x, emitter->startColor.y, emitter->startColor.z, emitter->startColor.w);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "endColor");
	emitter->endColor = *(glm::vec4*)lua_touserdata(luaState, -1);
	//PRINT("EndColor %f %f %f %f\n", emitter->endColor.x, emitter->endColor.y, emitter->endColor.z, emitter->endColor.w);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "force");
	emitter->force = *(glm::vec2*)lua_touserdata(luaState, -1);
	//PRINT("Force %f %f\n", emitter->force.x, emitter->force.y);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "spread");
        emitter->spread = (float)lua_tonumber(luaState, -1);
	//PRINT("Spread %f\n", emitter->spread);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "position");
	emitter->position = *(glm::vec2*)lua_touserdata(luaState, -1);
	//PRINT("Position %f %f\n", emitter->position.x, emitter->position.y);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "relative");
	emitter->relative = lua_toboolean(luaState, -1);
	//PRINT("Relative %d\n", emitter->relative);
	lua_pop(luaState, 1);

	lua_getfield(luaState, -1, "texture");
	emitter->texture = *(Texture**)lua_touserdata(luaState, -1);
	if(emitter->texture)
	    //PRINT("Texture %d\n", emitter->texture->handle);
	lua_pop(luaState, 1);
        
	lua_pop(luaState, 1);

	if(emitter->next == nullptr)
	    emitter->next = (ParticleEmitterDescription*)arena->alloc(sizeof(ParticleEmitterDescription));
	
	emitter = emitter->next;
    }
}

ParticleEffectDescription* ParticleEffectManager::load(lua_State* luaState)
{
    lua_getfield(luaState, 1, "name");
    const char* name = lua_tostring(luaState, -1);
    
    ParticleEffectDescription* result = effects.lookup(name);
    if(!result)
    {
	result = effects.add(name);
	if(!result)
	{
	    PRINT("Error adding effect %s. \n", name);
	    return result;
	}
	else
	{
	    storeEffectParams(result, luaState, stack);
	}
    }
    else
    {
	storeEffectParams(result, luaState, stack);
    }

    return result;
}

void ParticleEffectManager::remove(ParticleEffectDescription* effect)
{
    for(int i = 0; i < ARRAYSIZE(effects.map); ++i)
    {
	ResourceNode<ParticleEffectDescription>* node = effects.map + i;
	if(node->resource == effect)
	{
	    node->resource = nullptr;
	    node->hash = -1;
	    break;
	}
    }
}

void ParticleEffectManager::clean()
{
    for(int i = 0; i < ARRAYSIZE(effects.map); ++i)
    {
	ResourceNode<ParticleEffectDescription>* node = effects.map + i;
	node->resource = nullptr;
	node->hash = -1;
    }
}

ResourceManager::~ResourceManager()
{
    textures.clean();
    effects.clean();
}
  
