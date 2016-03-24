#include "../voidstorm_platform.h"
#include "../voidstorm_config.h"
#include <SDL2/SDL_syswm.h>

static void
printGetLastError()
{
	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0)
		printf("No error message recorded.\n");
	
	LPSTR messageBuffer = NULL;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	printf("%s\n", messageBuffer);
	LocalFree(messageBuffer);
}

bool
getLastWriteTime(char *filename, FileTime *filetime)
{
    WIN32_FIND_DATA findData;
    HANDLE handle = FindFirstFileA(filename, (LPWIN32_FIND_DATAA)&findData);
    if(handle != INVALID_HANDLE_VALUE)
    {
	filetime->time = findData.ftLastWriteTime;
	FindClose(handle);
	return true;
    }
    
    return false;
}

int
compareFileTime(FileTime& a, FileTime& b)
{
    return CompareFileTime(&a.time, &b.time);
}

static bool
initializeMemory(GameMemory *memory)
{
#ifdef _HAVE_X64
    INT_PTR BaseAddress = 0x00000000003d0000;
#else
    INT_PTR BaseAddress = 0;
#endif

    memory->permanentStorageSize = PERMANENT_STORAGE_SIZE;
    memory->transientStorageSize = TRANSIENT_STORAGE_SIZE;

    size_t totalSize = memory->permanentStorageSize + memory->transientStorageSize;
	
    memory->permanentStoragePtr = VirtualAlloc((LPVOID)BaseAddress, totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    if(memory->permanentStoragePtr == NULL)
    {
	printf("Failed to VirtuaAlloc.\n");
	printGetLastError();
	return false;
    }
	
    memory->transientStoragePtr = (uint8_t *)memory->permanentStoragePtr + memory->permanentStorageSize;

    return true;
}

static bool32
releaseMemory(GameMemory *memory)
{
    bool32 success = VirtualFree(
	memory->permanentStoragePtr,      
	0,             
	MEM_RELEASE);  

    return success; 
}

static bool
setupRecordingState(GameMemory *memory, RecordingState *state)
{
    state->isRecording = false;
    state->isPlayback = false;
   
    state->handle = CreateFileA("rstate_storage.imi", GENERIC_WRITE|GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);

    LARGE_INTEGER maxSize;
    maxSize.QuadPart = memory->permanentStorageSize;
    
    state->memoryMap = CreateFileMapping(
	state->handle, 0, PAGE_READWRITE,
	maxSize.HighPart, maxSize.LowPart, 0);

    state->memory = MapViewOfFile(state->memoryMap, FILE_MAP_ALL_ACCESS, 0, 0, memory->permanentStorageSize);
    if (state->memory == NULL)
    {
	printGetLastError();
	return false;
    }

    return true;
}

static void
endRecordingState(RecordingState *state)
{
    CloseHandle(state->handle);
}

static void
beginRecord(GameMemory *memory, RecordingState *state)
{
    if(state->isRecording || state->isPlayback)
	return;
    
    state->isRecording = true;
    
    state->recordHandle = CreateFile(L"rstate_input.imi", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (state->recordHandle == INVALID_HANDLE_VALUE)
    {
	printf("BeginRecord error.\n");
	printGetLastError();
	return;
    }
    
    printf("Begin recording.\n");
    
    CopyMemory(state->memory, memory->permanentStoragePtr, memory->permanentStorageSize);
}

static void
endRecord(RecordingState *state)
{
    if(!state->isRecording)
	return;
    
    state->isRecording = false;
    
    CloseHandle(state->recordHandle);
    printf("End recording.\n");
}

static void
beginPlayback(GameMemory *memory, RecordingState *state)
{
    if(state->isPlayback || state->isRecording)
	return;
    
    state->isPlayback = true;
    
    state->playbackHandle = CreateFile(L"rstate_input.imi", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if (state->playbackHandle == INVALID_HANDLE_VALUE)
    {
	printf("BeginPlayback error.\n");
	printGetLastError();
	return;
    }
    
    printf("Begin playback.\n");
    
    CopyMemory(memory->permanentStoragePtr, state->memory, memory->permanentStorageSize);
}

static void
endPlayback(RecordingState *state)
{
    if(!state->isPlayback)
	return;
    
    state->isPlayback = false;
    
    CloseHandle(state->playbackHandle);
    printf("End playback\n");
}

static void
recordInput(RecordingState *state, GameInput *input)
{
    DWORD bytesWritten;
    WriteFile(state->recordHandle, input, sizeof(*input), &bytesWritten, 0);
}

static bool
playbackInput(GameMemory *memory, RecordingState *state, GameInput *input)
{
    DWORD bytesRead = 0;
    if(ReadFile(state->playbackHandle, input, sizeof(GameInput), &bytesRead, 0))
    {
	if(bytesRead == 0)
	{
	    endPlayback(state);
	    beginPlayback(memory, state);
	    ReadFile(state->playbackHandle, input, sizeof(*input), &bytesRead, 0);
	    return true;
	}
    }
    return false;
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001%28v=vs.85%29.aspx#dead_zone
#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689

// NOTE (daniel): The value represents one axis within the range (-32768 to 32767)
// we zero out the value if within the deadzone and normalize it to 0.0 to 1.0 if outside.
// https://wiki.libsdl.org/SDL_ControllerAxisEvent
static float
processAxis(int value, int deadzone)
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

static State
handleEvents(ControllerState *controllerState, Controller *controller)
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
		//renderer->setResolution(glm::ivec2(event.window.data1, event.window.data2));
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

HeapAllocator* g_heapAllocator;

int main(int argv, char** argc)
{
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0)
    {
	printf("ERROR: Failed to initialize SDL %s\n", SDL_GetError());
	return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Voidstorm",
					  SDL_WINDOWPOS_UNDEFINED,
					  SDL_WINDOWPOS_UNDEFINED,
					  1280,
					  720,
					  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

   
    if (window == NULL)
    {
	printf("ERROR: Failed to create window %s \n", SDL_GetError());
	return 1;
    }
    else
    {
	GameMemory gameMemory = {};
	initializeMemory(&gameMemory);

	uint8_t *memoryPtr = (uint8_t *)gameMemory.permanentStoragePtr + APPLICATION_STORAGE_RESERVED_SIZE;
	
	mspace globalSpace = create_mspace_with_base(
	    memoryPtr,
	    VOIDSTORM_APPLICATION_HEAP_SIZE,
	    0);    
	HeapAllocator globalHeapAllocator(globalSpace);
	g_heapAllocator = &globalHeapAllocator;
	gameMemory.heapAllocator = &globalHeapAllocator;

	gameMemory.ms = globalSpace;

	mspace renderSpace = create_mspace_with_base(
	    (uint8_t*)memoryPtr + VOIDSTORM_APPLICATION_HEAP_SIZE,
	    VOIDSTORM_RENDER_HEAP_SIZE,
	    0);    
	HeapAllocator renderHeapAllocator(renderSpace);	
	
	uint8_t *permStackPtr = (uint8_t *)memoryPtr + VOIDSTORM_APPLICATION_HEAP_SIZE + VOIDSTORM_RENDER_HEAP_SIZE;	
	dcutil::StackAllocator *permStackAllocator = new(permStackPtr) dcutil::StackAllocator(permStackPtr + sizeof(dcutil::StackAllocator), VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE);
	gameMemory.permStackAllocator = permStackAllocator;

	uint8_t *gameStackPtr = (uint8_t *)permStackPtr + VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE;
	dcutil::StackAllocator *gameStackAllocator = new(gameStackPtr) dcutil::StackAllocator(gameStackPtr + sizeof(dcutil::StackAllocator), VOIDSTORM_APPLICATION_GAME_STACK_SIZE);
	gameMemory.gameStackAllocator = gameStackAllocator;
	    
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	HWND hwnd = info.info.win.window;
	
	dcfx::Context *renderContext = new(gameMemory.permStackAllocator->alloc(sizeof(dcfx::Context))) dcfx::Context(hwnd, &renderHeapAllocator);
    
	Controller activeController;  
	GameInput gameInput;
	gameInput.currentKeyboard = &gameInput.keyboards[0];
	gameInput.previousKeyboard = &gameInput.keyboards[1];
	gameInput.currentController = &gameInput.controllers[0];
	gameInput.previousController = &gameInput.controllers[1];

	State state = State::RUNNING;
	
	if(!initializeGame(&gameMemory, renderContext))
	{
	    printf("ERROR: Failed to initialze game\n");
	    state = State::QUIT;
	}
	
#ifdef VOIDSTORM_INTERNAL
	RecordingState rstate;
	if(!setupRecordingState(&gameMemory, &rstate))
	{
	    printf("ERROR: Failed to setup recording state\n");
	    state = State::QUIT;
	}
#endif
	uint64_t frameCounter = 0;
	uint64_t prevTime = SDL_GetPerformanceCounter();
	while(state == State::RUNNING)
	{
	    TIME_BLOCK(Frame);

	    uint64_t currentTime = SDL_GetPerformanceCounter();
	    gameInput.dt = (float)((currentTime - prevTime) / (double)SDL_GetPerformanceFrequency());
	    prevTime = currentTime;
	    
	    state = handleEvents(gameInput.currentController, &activeController);

	    gameInput.currentController->A.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_A);
	    gameInput.currentController->B.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_B);
	    gameInput.currentController->X.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_X);
	    gameInput.currentController->Y.isPressed = SDL_GameControllerGetButton(activeController.pad, SDL_CONTROLLER_BUTTON_Y);

	    memcpy(gameInput.currentKeyboard->keys, SDL_GetKeyboardState(NULL), sizeof(uint8_t) * SDL_NUM_SCANCODES);
	    
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
		    // This needs to be a callback into the game to decide what to do when
		    // a playback is completed.
		    // Reload the script files incase the scripts are modified during playback
/*		    
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
*/		    
		}
	    }
#endif

	    // Update game
	    updateGame(&gameMemory, &gameInput);
	    
	    KeyboardState* ktemp = gameInput.currentKeyboard;
	    gameInput.currentKeyboard = gameInput.previousKeyboard;
	    gameInput.previousKeyboard = ktemp;
	
	    ControllerState* temp = gameInput.currentController;
	    gameInput.currentController = gameInput.previousController;
	    gameInput.previousController = temp;

	    frameCounter++;
	    
	}

	endRecordingState(&rstate);

	shutdownGame(&gameMemory);
	
	renderContext->~Context();
	
	releaseMemory(&gameMemory);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    WorkThreadContext* context = (WorkThreadContext*)lpParameter;
    WorkQueue* queue = context->queue;

    for(;;)
    {
	if(doNextEntry(queue, context))
	{
	    WaitForSingleObjectEx(queue->semaphore, INFINITE, FALSE);
	}
    }
}

void
setupWorkQueue(WorkQueue *queue, uint32_t threadCount, WorkThreadContext *contexts)
{
    queue->completionGoal = 0;
    queue->completionCount = 0;
    queue->nextEntryToWrite = 0;
    queue->nextEntryToRead = 0;

    uint32_t initialCount = 0;
    queue->semaphore = CreateSemaphoreEx(0,
					 initialCount,
					 threadCount,
					 0, 0, SEMAPHORE_ALL_ACCESS);

    for(uint32_t i = 0; i < threadCount; ++i)
    {
	WorkThreadContext* context = contexts + i;
	context->queue = queue;
	
	DWORD threadID;
        HANDLE thread = CreateThread(0, 0, ThreadProc, context, 0, &threadID);
	CloseHandle(thread);
    }
}

void
addEntry(WorkQueue *queue, WorkQueueCallback *callback, void *data)
{
    // TODO (daniel): Add InterlockedCompareExchange so any thread can add.
    uint32_t newNextEntryToWrite = (queue->nextEntryToWrite + 1) % ARRAYSIZE(queue->entries);
    assert(newNextEntryToWrite != queue->nextEntryToRead);

    WorkQueueEntry *entry = &queue->entries[queue->nextEntryToWrite];
    entry->data = data;
    entry->callback = callback;

    _WriteBarrier();
    queue->nextEntryToWrite = newNextEntryToWrite;

    // Signal the queue
    ReleaseSemaphore(queue->semaphore, 1, 0);
}

bool
doNextEntry(WorkQueue *queue, WorkThreadContext *context)
{
    bool sleep = false;

    uint32_t originalEntryToRead = queue->nextEntryToRead;
    uint32_t newNextEntryToRead = (originalEntryToRead + 1) % ARRAYSIZE(queue->entries);
    if(originalEntryToRead != queue->nextEntryToWrite)
    {
	// Prevent two thread from reading the same entry
	uint32_t index = _InterlockedCompareExchange((LONG volatile*)&queue->nextEntryToRead,
						     newNextEntryToRead,
						     originalEntryToRead);

	if(index == originalEntryToRead)
	{	
	    WorkQueueEntry* entry = &queue->entries[index];
	    entry->callback(queue, entry->data, context);
	}
    }
    else
    {
        sleep = true;
    }

    return sleep;
}
