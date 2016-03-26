#include "voidstorm.h"
#include "voidstorm_alloc.cpp"
#include "voidstorm_resources.cpp"
#include "voidstorm_entity.cpp"
#include "voidstorm_component.cpp"
#include "voidstorm_spritebatch.cpp"
#include "voidstorm_linerender.cpp"
#include "voidstorm_particle.cpp"
#include "voidstorm_render.cpp"
#include "voidstorm_shape.cpp"
#include "voidstorm_collision.cpp"
#include "voidstorm_physics.cpp"
#include "voidstorm_api.cpp"

static_assert((VOIDSTORM_APPLICATION_HEAP_SIZE
	       + VOIDSTORM_RENDER_HEAP_SIZE
	       + VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE
	       + VOIDSTORM_APPLICATION_GAME_STACK_SIZE
	       + sizeof(VoidstormContext)) <= PERMANENT_STORAGE_SIZE,
	      "The application is asking for more memory then the platform layer supplies.");

#ifdef VOIDSTORM_INTERNAL
ProfileRecord g_counters[VOIDSTORM_PROFILE_NUM_COUNTERS];
tinystl::vector<LuaFile> *g_luaFiles;
#endif

static void
resetWorldManagers(World *world)
{
    world->entities->reset();
    world->transforms->data.used = 1;
    world->transforms->map = ComponentMap();
    world->physics->data.used = 1;
    world->physics->map = ComponentMap();
    world->collisions->reset();
    world->responders->data.used = 1;
    world->responders->map = ComponentMap();
    world->sprites->data.used = 1;
    world->sprites->map = ComponentMap();
}

WORK_QUEUE_CALLBACK(TestWorker)
{
    PRINT("Thread %d\n", *((int*)data));
}

bool
initializeGame(GameMemory *memory, dcfx::Context *renderContext)
{
    VoidstormContext *context = (VoidstormContext *)memory->permanentStoragePtr;

    context->queue = (WorkQueue *)memory->permStackAllocator->alloc(sizeof(WorkQueue));
    
    WorkThreadContext contexts[8];
    setupWorkQueue(context->queue, 8, contexts);

#if 0    
    int test = 5;
    addEntry(context.queue, TestWorker, &test);
#endif    
 
    context->simulator = new(memory->permStackAllocator->alloc(sizeof(PhysicsSimulator))) PhysicsSimulator(memory->permStackAllocator);
    context->renderer = new(memory->permStackAllocator->alloc(sizeof(Renderer))) Renderer(renderContext, memory);
    
    context->world.entities = new(memory->permStackAllocator->alloc(sizeof(EntityManager))) EntityManager();

    context->world.collisions = new(memory->permStackAllocator->alloc(sizeof(CollisionManager))) CollisionManager(memory->permStackAllocator);
    context->world.collisions->allocate(VOIDSTORM_COLLISION_COMPONENT_COUNT);

    context->world.transforms = new(memory->permStackAllocator->alloc(sizeof(CollisionManager))) TransformManager(memory->permStackAllocator);
    context->world.transforms->allocate(VOIDSTORM_TRANSFORM_COMPONENT_COUNT);

    context->world.physics = new(memory->permStackAllocator->alloc(sizeof(CollisionManager))) PhysicsManager(memory->permStackAllocator);
    context->world.physics->allocate(VOIDSTORM_PHYSICS_COMPONENT_COUNT);

    context->world.responders = new(memory->permStackAllocator->alloc(sizeof(CollisionManager))) CollisionResponderManager(memory->permStackAllocator);
    context->world.responders->allocate(VOIDSTORM_RESPONDER_COMPONENT_COUNT);

    context->world.sprites = new(memory->permStackAllocator->alloc(sizeof(CollisionManager))) SpriteManager(memory->permStackAllocator);
    context->world.sprites->allocate(VOIDSTORM_SPRITE_COMPONENT_COUNT);

    context->resources = new(memory->permStackAllocator->alloc(sizeof(ResourceManager))) ResourceManager(memory->permStackAllocator, renderContext);
    
    context->allocatorUserData.ms = memory->ms;
    context->allocatorUserData.pool = new(memory->permStackAllocator->alloc(sizeof(dcutil::PoolAllocator))) dcutil::PoolAllocator(
	memory->permStackAllocator->alloc(VOIDSTORM_SCRIPT_ELEMENT_SIZE * VOIDSTORM_SCRIPT_ELEMENT_COUNT),
	VOIDSTORM_SCRIPT_ELEMENT_SIZE,
	VOIDSTORM_SCRIPT_ELEMENT_COUNT);
	
    context->luaState = lua_newstate(LuaAllocatorCallback, &context->allocatorUserData);

#ifdef VOIDSTORM_INTERNAL
    g_luaFiles = &context->luaFiles;
#endif

    lua_State *luaState = context->luaState;
    
    luaL_openlibs(luaState);

    api::initialize(context->luaState);

    bool error = false;
    do
    {
	memory->gameStackAllocator->reset();

	resetWorldManagers(&context->world);
	
	context->renderer->getParticleEngine()->reset();

	// Pass the engine context to the lua state
	lua_getglobal(luaState, "_G");
	lua_pushlightuserdata(luaState, context);
	lua_setfield(luaState, -2, "state");

	lua_newtable(luaState);
	{
	    LUA_ENUM(luaState, debug, 0);
	    LUA_ENUM(luaState, release, 1);
	}
	lua_setfield(luaState, -2, "buildtype");

#ifdef _DEBUG
	lua_pushinteger(luaState, 0);
#else
	lua_pushinteger(luaState, 1);
#endif	
	lua_setfield(luaState, -2, "build");

	lua_getfield(luaState, -1, "dofile");
	lua_pushstring(luaState, "voidstorm.lua");
	if(lua_pcall(luaState, 1, 0, 0) != 0)
	{
	    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
	    PRINT( "Press any key to continue..." );
	    getchar();
	    error = true;
	}  else { error = false; }
	    
	lua_getfield(luaState, -1, "GameInitialize");
	if(lua_pcall(luaState, 0, 0, 0) != 0)
	{
	    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
	    PRINT( "Press any key to continue..." );
	    getchar();
	    error = true;
	} else { error = false; }

	lua_pop(luaState, 1);
        
    } while (error);

    return true;
}

int
updateGame(GameMemory* memory, GameInput *input)
{
    VoidstormContext *context = (VoidstormContext *)memory->permanentStoragePtr;
    
    lua_State *luaState = context->luaState;
    World *world = &context->world;
    PhysicsSimulator *simulator = context->simulator;
    Renderer *renderer = context->renderer;

    // We could store this once in initialize instead..
    context->input = input;
    
    if(isKeyDownToReleased(input, VOIDSTORM_KEY_F11))
    {
	//renderer->toogleFullscreen();
    }
	
#ifdef VOIDSTORM_INTERNAL	
    if(isKeyDownToReleased(input, 'R'))
    {
	memory->gameStackAllocator->reset();

	resetWorldManagers(&context->world);
	
	renderer->getParticleEngine()->reset();
		
	lua_getglobal(luaState, "_G");
	lua_getfield(luaState, -1, "GameInitialize");

	if(lua_pcall(luaState, 0, 0, 0) != 0)
	{
	    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
	}
	lua_pop(luaState, 1);
    }

    for (size_t i = 0; i < context->luaFiles.size(); ++i)
    {	
	LuaFile& file = context->luaFiles[i];

	FileTime currentFileTime;
	if(getLastWriteTime(file.path, &currentFileTime))
	{		
	    if(compareFileTime(file.timeWhenLoaded, currentFileTime) != 0)
	    {
		lua_getfield(luaState, LUA_GLOBALSINDEX, "dofile");
		lua_pushstring(luaState, file.name);

		if(lua_pcall(luaState, 1, 0, 0) != 0)
		{
		    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
		}
	    }
	}
    }

#if VOIDSTORM_DEBUG_SPRITE_BOUNDS == 1	    	    
    for(uint32_t i = 1; i < world->sprites.data.used; ++i)
    {
	Entity e = world->sprites.data.entities[i];
	TransformManager::Instance transform = world->transforms.lookup(e);
		
	glm::vec2 pos = world->transforms.data.position[transform.index];
	glm::vec2 size = world->sprites.data.size[i];
	glm::vec2 origin = world->sprites.data.origin[i];

	AABB aabb;
	aabb.lower = pos - origin * size;
	aabb.upper = pos + size - origin * size;

	renderer->getLineRenderer()->drawAABB(aabb, glm::vec4(1,0,0,1));
    }
#endif	    
	    
#if VOIDSTORM_DEBUG_COLLISION_BOUNDS == 1
    for(uint32_t i = 1; i < world->collisions.data.used; ++i)
    {
	Entity e = world->collisions.data.entities[i];
	TransformManager::Instance transform = world->transforms.lookup(e);
	glm::vec2 pos = world->transforms.data.position[transform.index];
	float rotation = world->transforms.data.rotation[transform.index];

	Transform t(pos, rotation);

	CollisionManager::ShapeData shapeData = world->collisions.data.shape[i];
	DbvtNode* node = world->collisions.data.node[i];
		
	if(shapeData.shape == ShapeType::CIRCLE)
	    renderer->getLineRenderer()->drawCircle(pos, shapeData.data.circle->radius, glm::vec4(0,0,1,1));

	if(shapeData.shape == ShapeType::POLYGON)
	    renderer->getLineRenderer()->drawPolygon(*shapeData.data.polygon, glm::vec4(0, 0, 1, 1), t);
		
#if VOIDSTORM_DEBUG_PROXY_BOUNDS == 1
	renderer->getLineRenderer()->drawAABB(node->aabb, glm::vec4(0,1,0,1));
#endif
    }
#endif	    
#endif	

    simulator->update(world, input->dt, NULL);
	    
    {
	TIME_BLOCK(LuaUpdateAndRender);

	lua_getglobal(luaState, "_G");

	lua_pushnumber(luaState, input->dt);
	lua_setfield(luaState, -2, "dt");
	
	lua_getfield(luaState, -1, "GameUpdateAndRender");
	lua_remove(luaState, -2);
	    
	if(lua_pcall(luaState, 0, 0, 0) != 0)
	{
	    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
	}
    }

    renderer->getParticleEngine()->update(input->dt);

    glm::vec2* currentPosition = world->transforms->data.position;
	    
    world->transforms->data.position = simulator->getInterpolatedState().position;
	    
    renderer->render(world);

    world->transforms->data.position = currentPosition;

    renderer->frame();
	
#ifdef VOIDSTORM_INTERNAL
    char messageOutput[100];
	
    static_assert(ARRAYSIZE(g_counters) > __COUNTER__, "The number of profile counters exceed the amount defined in voidstorm_config.h");
	
    for(int i = 0; i < __COUNTER__ - 1; ++i)
    {
	ProfileRecord record = g_counters[i];

	sprintf(messageOutput, "%s - %f ms", record.name, record.time);
	renderer->write(messageOutput, glm::vec2(0.8f, 0.025f * i + 0.01f), false);
    }
#endif
    
    return 0;
}

int
shutdownGame(GameMemory *memory)
{
    VoidstormContext *context = (VoidstormContext *)memory->permanentStoragePtr;
    
    lua_close(context->luaState);

    context->renderer->~Renderer();
    context->simulator->~PhysicsSimulator();
    context->resources->~ResourceManager();

    return 0;
}
