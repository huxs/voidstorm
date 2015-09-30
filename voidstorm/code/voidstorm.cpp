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

// TODO: Add reference for these values.
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689

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
	    
	case SDL_CONTROLLERDEVICEADDED:
	{
	    if(controller->pad == NULL)
	    {
		SDL_ControllerDeviceEvent& ev = event.cdevice;
		controller->pad = SDL_GameControllerOpen(ev.which);
		controller->index = ev.which;
		PRINT("Controller added.\n");
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
		PRINT("Controller removed.\n");
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
	dcutil::Stack* permanentStack = new(permStackPtr) dcutil::Stack(
	    permStackPtr + sizeof(dcutil::Stack),
	    VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE);

	HeapAllocator renderAllocator(renderSpace);	
	Renderer* renderer = new(permanentStack->alloc(sizeof(Renderer))) Renderer(&renderAllocator, permanentStack);
	
	ResourceManager* resources = new(permanentStack->alloc(sizeof(ResourceManager))) ResourceManager(permanentStack, renderer->getContext());
	PhysicsWorld* physics = new(permanentStack->alloc(sizeof(PhysicsWorld))) PhysicsWorld(permanentStack, renderer->getLineRenderer());

	World* world = new(permanentStack->alloc(sizeof(World))) World(permanentStack);
	world->transforms.allocate(VOIDSTORM_TRANSFORM_COMPONENT_COUNT);
	world->physics.allocate(VOIDSTORM_PHYSICS_COMPONENT_COUNT);
	world->collisions.allocate(VOIDSTORM_COLLISION_COMPONENT_COUNT);
	world->responders.allocate(VOIDSTORM_RESPONDER_COMPONENT_COUNT);
	world->sprites.allocate(VOIDSTORM_SPRITE_COMPONENT_COUNT);

#ifdef VOIDSTORM_INTERNAL
	tinystl::vector<LuaFile> luaFiles;
	g_luaFiles = &luaFiles;
#endif
    
	LuaAllocatorUserData allocatorUd;
	allocatorUd.pool = new(permanentStack->alloc(sizeof(dcutil::Pool))) dcutil::Pool(
	    permanentStack->alloc(VOIDSTORM_SCRIPT_ELEMENT_SIZE * VOIDSTORM_SCRIPT_ELEMENT_COUNT),
	    VOIDSTORM_SCRIPT_ELEMENT_SIZE,
	    VOIDSTORM_SCRIPT_ELEMENT_COUNT);
	
	allocatorUd.ms = appSpace;
    
	lua_State* luaState = lua_newstate(LuaAllocatorCallback, &allocatorUd);    
	luaL_openlibs(luaState);

	// NOTE: Register C-callbacks for lua.
	api::initialize(luaState);

	setupContactCallbacks();
	
	Controller activeController;
    
	GameInput gameInput;
	gameInput.currentKeyboard = &gameInput.keyboards[0];
	gameInput.previousKeyboard = &gameInput.keyboards[1];
	gameInput.currentController = &gameInput.controllers[0];
	gameInput.previousController = &gameInput.controllers[1];

	GameState gameState;
	gameState.renderer = renderer;
	gameState.physics = physics;
	gameState.resources = resources;
	gameState.input = &gameInput;
	gameState.world = world;

	lua_getglobal(luaState, "_G");
	lua_pushlightuserdata(luaState, &gameState);
	lua_setfield(luaState, -2, "state");
	lua_pushboolean(luaState, false);
	lua_setfield(luaState, -2, "initialized");

	lua_newtable(luaState);
	{
	    LUA_ENUM(luaState, debug, 0);
	    LUA_ENUM(luaState, release , 1);
	}
	lua_setfield(luaState, -2, "buildtype");
    
#ifdef _DEBUG
	lua_pushinteger(luaState, 0);
#else
	lua_pushinteger(luaState, 1);
#endif	
	lua_setfield(luaState, -2, "build");
    
	lua_getfield(luaState, -1, "dofile");
	lua_pushstring(luaState, "voidstorm_def.lua");

	if(lua_pcall(luaState, 1, 0, 0) != 0)
	{
	    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
	}

	lua_getfield(luaState, -1, "dofile");
	lua_pushstring(luaState, "voidstorm.lua");

	if(lua_pcall(luaState, 1, 0, 0) != 0)
	{
	    PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
	}

	lua_pop(luaState, 1);

	State state = State::RUNNING;
	bool32 initialized = true;   

	RecordingState rstate;
	if(!setupRecordingState(&gameMemory, &rstate))
	{
	    PRINT("Failed to setup recording state.\n");
	    //state = State::QUIT;
	}

	uint64_t frameCounter = 0;
	uint64_t prevTime = SDL_GetPerformanceCounter();
	while(state == State::RUNNING)
	{
	    TIME_BLOCK(Frame)
	
		uint64_t currentTime = SDL_GetPerformanceCounter();
	    gameInput.dt = (float)((currentTime - prevTime) / (double)SDL_GetPerformanceFrequency());
	    prevTime = currentTime;

	    //gameInput.dt = 1 / 60.0f;
	    state = handleEvents(gameInput.currentController, &activeController, renderer);

	    gameInput.currentController->A.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_A);
	    gameInput.currentController->B.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_B);
	    gameInput.currentController->X.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_X);
	    gameInput.currentController->Y.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_Y);

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
	
	    if(rstate.isRecording)
	    {
		recordInput(&rstate, &gameInput);
	    }

	    if(rstate.isPlayback)
	    {
		if(playbackInput(&gameMemory, &rstate, &gameInput))
		{
		    // NOTE: Reload the script files incase the scripts are modified
		    // during playback.
		    luaFiles.clear();
		    
		    lua_getfield(luaState, LUA_GLOBALSINDEX, "dofile");
		    lua_pushstring(luaState, "voidstorm.lua");

		    if (lua_pcall(luaState, 1, 0, 0) != 0)
		    {
			PRINT("lua_pcall: %s\n", lua_tostring(luaState, -1));
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
	    int b = (int)permanentStack->getTotalSize() / Megabytes(1);
	
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

