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
#include "voidstorm_physics.cpp"
#include "voidstorm_api.cpp"

enum State
{
    RUNNING,
    QUIT
};

struct Controller
{
    Controller()
	    : pad(NULL) {};
	
    SDL_GameController* pad;
    int index;
};

// https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001%28v=vs.85%29.aspx#dead_zone
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689

// NOTE (daniel): The value represents one axis within the range (-32768 to 32767)
// we zero out the value if within the deadzone and normalize it to 0.0 to 1.0 if outside.
// https://wiki.libsdl.org/SDL_ControllerAxisEvent
static float processAxis(int value, int deadzone)
{
    float result = 0.0f;

    if (value < -deadzone)
    {
	if (value < -32767) value = -32767;

	result = ((float)(value + deadzone)) / (32767 - deadzone);
    }
    else if (value > deadzone)
    {
	if (value > 32767) value = 32767;

	result = ((float)(value - deadzone)) / (32767 - deadzone);
    }

    return result;
};

static State handleEvents(ControllerState* controllerState, Controller* controller, Renderer* renderer)
{
    State state = State::RUNNING;
	
    SDL_Event event;
    while(SDL_PollEvent(&event)) 
    {
	switch(event.type) 
	{
	case SDL_QUIT:
	    state = State::QUIT;
	    break;
	case SDL_WINDOWEVENT:
	{
	    switch(event.window.event)
	    {
		/*case SDL_WINDOWEVENT_SHOWN:
		  SDL_Log("Window %d shown", event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_HIDDEN:
		  SDL_Log("Window %d hidden", event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_EXPOSED:
		  SDL_Log("Window %d exposed", event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_MOVED:
		  SDL_Log("Window %d moved to %d,%d",
		  event.window.windowID, event.window.data1,
		  event.window.data2);
		  break;*/
	    case SDL_WINDOWEVENT_RESIZED:
		SDL_Log("Window %d resized to %dx%d",
			event.window.windowID, event.window.data1,
			event.window.data2);
		renderer->setResolution(glm::ivec2(event.window.data1, event.window.data2));
		break;
		/*case SDL_WINDOWEVENT_MINIMIZED:
		  SDL_Log("Window %d minimized", event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_MAXIMIZED:
		  SDL_Log("Window %d maximized", event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_RESTORED:
		  SDL_Log("Window %d restored", event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_ENTER:
		  SDL_Log("Mouse entered window %d",
		  event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_LEAVE:
		  SDL_Log("Mouse left window %d", event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_FOCUS_GAINED:
		  SDL_Log("Window %d gained keyboard focus",
		  event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_FOCUS_LOST:
		  SDL_Log("Window %d lost keyboard focus",
		  event.window.windowID);
		  break;
		  case SDL_WINDOWEVENT_CLOSE:
		  SDL_Log("Window %d closed", event.window.windowID);
		  break;*/
	    }
	}
	break;
	case SDL_CONTROLLERDEVICEADDED:
	{
	    if(controller->pad == NULL)
	    {
		SDL_ControllerDeviceEvent& ev = event.cdevice;
		controller->pad = SDL_GameControllerOpen(ev.which);
		controller->index = ev.which;
	    }
	}
	break;
	case SDL_CONTROLLERDEVICEREMOVED:
	{
	    SDL_ControllerDeviceEvent& ev = event.cdevice;
	    if(controller->index == ev.which)
	    {
		SDL_GameControllerClose(controller->pad);
		controller->pad = NULL;
		controller->index = -1;
	    }
	}
	break;
	case SDL_CONTROLLERAXISMOTION:
	{
	    SDL_ControllerAxisEvent& ev = event.caxis;

	    switch(ev.axis)
	    {
	    case SDL_CONTROLLER_AXIS_LEFTX:
		controllerState->leftStickX = processAxis(ev.value, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		break;
	    case SDL_CONTROLLER_AXIS_LEFTY:
		controllerState->leftStickY = processAxis(ev.value, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		break;
	    case SDL_CONTROLLER_AXIS_RIGHTX:
		controllerState->rightStickX = processAxis(ev.value, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		break;
	    case SDL_CONTROLLER_AXIS_RIGHTY:
		controllerState->rightStickY = processAxis(ev.value, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		break;
	    };	
	}
	break;
	}
    }

    return state;
}

#ifdef VOIDSTORM_INTERNAL
ProfileRecord g_counters[VOIDSTORM_PROFILE_NUM_COUNTERS];
tinystl::vector<LuaFile>* g_luaFiles;
#endif

HeapAllocator* g_allocator;

int main(int argv, char** argc)
{
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0)
    {
	PRINT("Failed to initialize SDL %s.\n", SDL_GetError());
	return 1;
    }
    
    GameMemory gameMemory = {};
    initializeMemory(&gameMemory);

    {    
	mspace appSpace = create_mspace_with_base(gameMemory.permanentStorage, VOIDSTORM_APPLICATION_HEAP_SIZE, 0);
	mspace renderSpace = create_mspace_with_base((uint8_t*)gameMemory.permanentStorage + VOIDSTORM_APPLICATION_HEAP_SIZE, VOIDSTORM_RENDER_HEAP_SIZE, 0);
	
	HeapAllocator globalAllocator(appSpace);
	g_allocator = &globalAllocator;

	uint8_t* permStackPtr = (uint8_t*)gameMemory.permanentStorage + VOIDSTORM_APPLICATION_HEAP_SIZE + VOIDSTORM_RENDER_HEAP_SIZE;	

	dcutil::StackAllocator* permanentStack = new(permStackPtr) dcutil::StackAllocator(
	    permStackPtr + sizeof(dcutil::StackAllocator),
	    VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE);

	uint8_t* worldStackPtr = (uint8_t*)permStackPtr + VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE;
	dcutil::StackAllocator* worldStack = new(worldStackPtr) dcutil::StackAllocator(worldStackPtr + sizeof(dcutil::StackAllocator),
										       VOIDSTORM_APPLICATION_WORLD_STACK_SIZE);

	HeapAllocator renderAllocator(renderSpace);	
	Renderer* renderer = new(permanentStack->alloc(sizeof(Renderer))) Renderer(&renderAllocator, permanentStack, worldStack);
	
	World* world = new(permanentStack->alloc(sizeof(World))) World(permanentStack);
	world->transforms.allocate(VOIDSTORM_TRANSFORM_COMPONENT_COUNT);
	world->physics.allocate(VOIDSTORM_PHYSICS_COMPONENT_COUNT);
	world->collisions.allocate(VOIDSTORM_COLLISION_COMPONENT_COUNT);
	world->responders.allocate(VOIDSTORM_RESPONDER_COMPONENT_COUNT);
	world->sprites.allocate(VOIDSTORM_SPRITE_COMPONENT_COUNT);

	ResourceManager* resources = new(permanentStack->alloc(sizeof(ResourceManager))) ResourceManager(permanentStack, renderer->getContext());
	PhysicsWorld* physics = new(permanentStack->alloc(sizeof(PhysicsWorld))) PhysicsWorld(permanentStack, renderer->getLineRenderer());

#ifdef VOIDSTORM_INTERNAL
	tinystl::vector<LuaFile> luaFiles;
	g_luaFiles = &luaFiles;
#endif
    
	LuaAllocatorUserData allocatorUd;
	allocatorUd.pool = new(permanentStack->alloc(sizeof(dcutil::PoolAllocator))) dcutil::PoolAllocator(
	    permanentStack->alloc(VOIDSTORM_SCRIPT_ELEMENT_SIZE * VOIDSTORM_SCRIPT_ELEMENT_COUNT),
	    VOIDSTORM_SCRIPT_ELEMENT_SIZE,
	    VOIDSTORM_SCRIPT_ELEMENT_COUNT);
	
	allocatorUd.ms = appSpace;
    
	lua_State* luaState = lua_newstate(LuaAllocatorCallback, &allocatorUd);    
	luaL_openlibs(luaState);

	api::initialize(luaState);

	setupContactCallbacks();
	
	Controller activeController;  
	GameInput gameInput;
	gameInput.currentKeyboard = &gameInput.keyboards[0];
	gameInput.previousKeyboard = &gameInput.keyboards[1];
	gameInput.currentController = &gameInput.controllers[0];
	gameInput.previousController = &gameInput.controllers[1];

	VoidstormContext context;
	context.renderer = renderer;
	context.physics = physics;
	context.resources = resources;
	context.input = &gameInput;
	context.world = world;

	bool error = false;
	do
	{
	    worldStack->reset();
	    world->reset();
	    renderer->getParticleEngine()->reset();
	    
	    lua_getglobal(luaState, "_G");
	    lua_pushlightuserdata(luaState, &context);
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

	State state = State::RUNNING;
	
	RecordingState rstate;
	if(!setupRecordingState(&gameMemory, &rstate))
	{
	    PRINT("Failed to setup recording state.\n");
	    state = State::QUIT;
	}

	uint64_t frameCounter = 0;
	uint64_t prevTime = SDL_GetPerformanceCounter();
	while(state == State::RUNNING)
	{
	    TIME_BLOCK(Frame);
	
	    uint64_t currentTime = SDL_GetPerformanceCounter();
	    gameInput.dt = (float)((currentTime - prevTime) / (double)SDL_GetPerformanceFrequency());
	    prevTime = currentTime;

		//printf("dt %f\n", gameInput.dt);

	    //gameInput.currentController->rightStickX = 0.0f;
	    //gameInput.currentController->rightStickY = 0.0f;
	    //gameInput.dt = 1 / 60.0f;
	    
	    state = handleEvents(gameInput.currentController, &activeController, renderer);

	    gameInput.currentController->A.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_A);
	    gameInput.currentController->B.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_B);
	    gameInput.currentController->X.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_X);
	    gameInput.currentController->Y.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_Y);

	    memcpy(gameInput.currentKeyboard->keys, SDL_GetKeyboardState(NULL), sizeof(uint8_t) * SDL_NUM_SCANCODES);

	    if(isKeyDownAndReleased(&gameInput, SDL_SCANCODE_F11))
	    {
		renderer->toogleFullscreen();
	    }
	
#ifdef VOIDSTORM_INTERNAL	
	    if(isKeyDownAndReleased(&gameInput, SDL_SCANCODE_Q))
	    {
		beginRecord(&gameMemory, &rstate);
	    }	

	    if(isKeyDownAndReleased(&gameInput, SDL_SCANCODE_W))
	    {
		endRecord(&rstate);
	    }	

	    if(isKeyDownAndReleased(&gameInput, SDL_SCANCODE_O))
	    {
		beginPlayback(&gameMemory, &rstate);
	    }

	    if(isKeyDownAndReleased(&gameInput, SDL_SCANCODE_P))
	    {
		endPlayback(&rstate);
	    }

	    if(isKeyDownAndReleased(&gameInput, SDL_SCANCODE_R))
	    {
		worldStack->reset();
		world->reset();
		renderer->getParticleEngine()->reset();
		
		lua_getglobal(luaState, "_G");
		lua_getfield(luaState, -1, "GameInitialize");

		if(lua_pcall(luaState, 0, 0, 0) != 0)
		{
		    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
		}
		lua_pop(luaState, 1);
	    }
	    
	    if(rstate.isRecording)
	    {
		recordInput(&rstate, &gameInput);
	    }

	    if(rstate.isPlayback)
	    {
		if(playbackInput(&gameMemory, &rstate, &gameInput))
		{
		    // NOTE (daniel): Reload the script files incase the scripts are modified during playback
		    for (size_t i = 0; i < luaFiles.size(); ++i)
		    {
			LuaFile& file = luaFiles[i];
			
			lua_getfield(luaState, LUA_GLOBALSINDEX, "dofile");
			lua_pushstring(luaState, file.name);

			if(lua_pcall(luaState, 1, 0, 0) != 0)
			{
			    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
			}
		    }
		}
	    }

	    for (size_t i = 0; i < luaFiles.size(); ++i)
	    {
		LuaFile& file = luaFiles[i];

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
		
		CollisionManager::ShapeData shapeData = world->collisions.data.shape[i];
		DbvtNode* node = world->collisions.data.node[i];
		
		if(shapeData.shape == ShapeType::CIRCLE)
		    renderer->getLineRenderer()->drawCircle(pos, shapeData.data.circle->radius, glm::vec4(0,0,1,1));

		if(shapeData.shape == ShapeType::POLYGON)
		    renderer->getLineRenderer()->drawPolygon(*shapeData.data.polygon, glm::vec4(0, 0, 1, 1));
		
#if VOIDSTORM_DEBUG_PROXY_BOUNDS == 1
		renderer->getLineRenderer()->drawAABB(node->aabb, glm::vec4(0,1,0,1));
#endif
	    }
#endif	    
#endif	

	    {
		TIME_BLOCK(LuaUpdateAndRender);

		lua_getglobal(luaState, "_G");

		lua_pushnumber(luaState, gameInput.dt);
		lua_setfield(luaState, -2, "dt");
	
		lua_getfield(luaState, -1, "GameUpdateAndRender");
		lua_remove(luaState, -2);
	    
		if(lua_pcall(luaState, 0, 0, 0) != 0)
		{
		    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
		}
	    }

	    physics->simulate(world, gameInput.dt);

	    renderer->getParticleEngine()->update(gameInput.dt);

	    renderer->render(world);

	    renderer->frame();
	
#ifdef VOIDSTORM_INTERNAL
	    char messageOutput[100];
	
	    static_assert(ARRAYSIZE(g_counters) > __COUNTER__, "The number of profile counters exceed the amount defined in voidstorm_config.h");
	
	    for(int i = 0; i < __COUNTER__ - 1; ++i)
	    {
		ProfileRecord record = g_counters[i];

		sprintf(messageOutput, "%s - %f ms", record.name, record.time);
		renderer->write(messageOutput, glm::vec2(1000, 20 * i + 10), false);
	    }

	    int a = (int)permanentStack->getAllocatedSize() / Megabytes(1);
	    int b = (int)permanentStack->m_size / Megabytes(1);
	
	    sprintf(messageOutput, "Permanent Stack Used:%d Mb Size:%d Mb %0.2f%%\n", a, b, (a/(float)b)*100.0f);
	    renderer->write(messageOutput, glm::vec2(500, 10), false);
#endif
	    
	    KeyboardState* ktemp = gameInput.currentKeyboard;
	    gameInput.currentKeyboard = gameInput.previousKeyboard;
	    gameInput.previousKeyboard = ktemp;
	
	    ControllerState* temp = gameInput.currentController;
	    gameInput.currentController = gameInput.previousController;
	    gameInput.previousController = temp;

	    frameCounter++;
	}

	endRecordingState(&rstate);
    
	lua_close(luaState);

	world->~World();
	resources->~ResourceManager();
	renderer->~Renderer();
    }
    
    releaseMemory(&gameMemory);
    
    SDL_Quit();
    return 0;
}

