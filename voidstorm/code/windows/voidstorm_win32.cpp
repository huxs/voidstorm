#include "../voidstorm_platform.h"
#include "../voidstorm_config.h"

#include <new>
#include <intrin.h>
#include <xinput.h>
#include <assert.h>

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

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(XInputGetStateProc);
X_INPUT_GET_STATE(xinputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

XInputGetStateProc *xinputGetState = xinputGetStateStub;

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(XInputSetStateProc);
X_INPUT_SET_STATE(xinputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

XInputSetStateProc *xinputSetState = xinputSetStateStub;

static void
loadXInput()
{
    HMODULE xinputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!xinputLibrary)
    {
	xinputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }

    if(!xinputLibrary)
    {
	xinputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    if(xinputLibrary)
    {
	xinputGetState = (XInputGetStateProc *)GetProcAddress(xinputLibrary, "XInputGetState");
	if(!xinputGetState) { xinputGetState = xinputGetStateStub; }

	xinputSetState = (XInputSetStateProc *)GetProcAddress(xinputLibrary, "XInputSetState");
	if(!xinputSetState) { xinputSetState = xinputSetStateStub; }
    }
    else
    {
	printf("ERROR: Failed to load XInput\n");
    }

}

enum State
{
    RUNNING,
    QUIT
};

HeapAllocator *g_heapAllocator;
bool g_showCursor;
State g_state;

static LRESULT CALLBACK
wndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{       
    LRESULT result = 0;

    switch(msg)
    {
    case WM_QUIT:
    case WM_CLOSE:
    case WM_DESTROY:
    {
	PostQuitMessage(0);
	g_state = State::QUIT;
    } break;

    case WM_SETCURSOR:
    {
	if(g_showCursor)
	{
	    result = DefWindowProcA(window, msg, wparam, lparam);
	}
	else
	{
	    SetCursor(0);
	}
    } break;

    case WM_ACTIVATEAPP:
    {
	if(wparam == TRUE)
	{
	    SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_ALPHA);
	}
	else
	{
	    SetLayeredWindowAttributes(window, RGB(0, 0, 0), 64, LWA_ALPHA);
	}
    } break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
    } break;

    default:
    {
	result = DefWindowProcA(window, msg, wparam, lparam);
    } break;
    }

    return result;
}

static void
processPendingMessages(KeyboardState *keyboardState, GameMemory *memory, RecordingState *rstate)
{
    WaitForInputIdle(GetCurrentProcess(), 16);
    
    MSG msg;
    while (0 != PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
    {
	switch(msg.message)
        {
	case WM_QUIT:
	{
	    g_state = State::QUIT;
	} break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
	    uint32_t vkCode = (uint32_t)msg.wParam;
	    
	    bool32 wasDown = ((msg.lParam & (1 << 30)) != 0);
	    bool32 isDown = ((msg.lParam & (1 << 31)) == 0);

	    //printf("Key %d is down %d\n", vkCode, isDown);
	    keyboardState->keys[vkCode] = isDown;
	    
	} break;

	default:
	{
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	} break;
	}
    }

}

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmd, int show)
{
    g_showCursor = true;
    
    loadXInput();
    
    WNDCLASSA windowClass = {};

    windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    windowClass.lpfnWndProc = wndProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.lpszClassName = "VoidstormWindowClass";

    if(RegisterClassA(&windowClass))
    {
	RECT windowRect = { 0, 0, static_cast<LONG>(1280), static_cast<LONG>(720) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	
        HWND hwnd = CreateWindowExA(
	    NULL, // WS_EX_TOPMOST|WS_EX_LAYERED,
	    windowClass.lpszClassName,
	    "Voidstorm",
	    WS_OVERLAPPEDWINDOW,
	    CW_USEDEFAULT,
	    CW_USEDEFAULT,
	    windowRect.right - windowRect.left,
	    windowRect.bottom - windowRect.top,
	    NULL,
	    NULL,
	    instance,
	    NULL);
    
        if(hwnd)
	{
	    ShowWindow(hwnd, 10);
	    
	    GameMemory memory = {};
	    initializeMemory(&memory);

	    uint8_t *memoryPtr = (uint8_t *)memory.permanentStoragePtr + GAME_STORAGE_RESERVED_SIZE;
	
	    mspace globalSpace = create_mspace_with_base(memoryPtr, VOIDSTORM_APPLICATION_HEAP_SIZE, 0);    
	    HeapAllocator globalHeapAllocator(globalSpace);
	    memory.globalHeapAllocator = &globalHeapAllocator;

	    mspace renderSpace = create_mspace_with_base((uint8_t*)memoryPtr + VOIDSTORM_APPLICATION_HEAP_SIZE, VOIDSTORM_RENDER_HEAP_SIZE, 0);    
	    HeapAllocator renderHeapAllocator(renderSpace);
	    memory.renderHeapAllocator = &renderHeapAllocator;
	
	    uint8_t *permStackPtr = (uint8_t *)memoryPtr + VOIDSTORM_APPLICATION_HEAP_SIZE + VOIDSTORM_RENDER_HEAP_SIZE;	
	    dcutil::StackAllocator *permStackAllocator = new(permStackPtr) dcutil::StackAllocator(permStackPtr + sizeof(dcutil::StackAllocator), VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE);
	    memory.permStackAllocator = permStackAllocator;

	    uint8_t *gameStackPtr = (uint8_t *)permStackPtr + VOIDSTORM_APPLICATION_PERMANENT_STACK_SIZE;
	    dcutil::StackAllocator *gameStackAllocator = new(gameStackPtr) dcutil::StackAllocator(gameStackPtr + sizeof(dcutil::StackAllocator), VOIDSTORM_APPLICATION_GAME_STACK_SIZE);
	    memory.gameStackAllocator = gameStackAllocator;

	    g_heapAllocator = &globalHeapAllocator;

	    GameInput input;
	    input.currentKeyboard = &input.keyboards[0];
	    input.previousKeyboard = &input.keyboards[1];
	    input.currentController = &input.controllers[0];
	    input.previousController = &input.controllers[1];

	    g_state = State::RUNNING;
	
	    if(!initializeGame(&memory, hwnd))
	    {
		printf("ERROR: Failed to initialze game\n");
		g_state = State::QUIT;
	    }
	
#ifdef VOIDSTORM_INTERNAL
	    RecordingState rstate;
	    if(!setupRecordingState(&memory, &rstate))
	    {
		printf("ERROR: Failed to setup recording state\n");
		g_state = State::QUIT;
	    }
#endif
	    uint64_t frameCounter = 0;
	    uint64_t prevTime = getPerformanceCounter();
	    while(g_state == State::RUNNING)
	    {
		TIME_BLOCK(Frame);

		uint64_t currentTime = getPerformanceCounter();
		input.dt = (float)((currentTime - prevTime) / (double)getPerformanceFrequency());
		prevTime = currentTime;

		processPendingMessages(input.currentKeyboard, &memory, &rstate);

		// Simple solution to query just the first controller
		XINPUT_STATE controllerState;
		if(xinputGetState(0, &controllerState) == ERROR_SUCCESS)
		{
		    XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

		    input.currentController->leftStickX = processAxis(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		    input.currentController->leftStickY = -processAxis(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		    input.currentController->rightStickX = processAxis(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		    input.currentController->rightStickY = -processAxis(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		    
		    input.currentController->A.isPressed = ((pad->wButtons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A);
		    input.currentController->B.isPressed = ((pad->wButtons & XINPUT_GAMEPAD_B) == XINPUT_GAMEPAD_B);
		    input.currentController->X.isPressed = ((pad->wButtons & XINPUT_GAMEPAD_X) == XINPUT_GAMEPAD_X);
		    input.currentController->Y.isPressed = ((pad->wButtons & XINPUT_GAMEPAD_Y) == XINPUT_GAMEPAD_Y);
		}
		else
		{
		    // Controller disconected
		}

#ifdef VOIDSTORM_INTERNAL	    
		if(isKeyDownToReleased(&input, 'Q'))
		{
		    beginRecord(&memory, &rstate);
		}	

		if(isKeyDownToReleased(&input, 'W'))
		{
		    endRecord(&rstate);
		}	

		if(isKeyDownToReleased(&input, 'O'))
		{
		    beginPlayback(&memory, &rstate);
		}

		if(isKeyDown(&input, 'P'))
		{
		    endPlayback(&rstate);
		}

		if(rstate.isRecording)
		{
		    recordInput(&rstate, &input);
		}

		if(rstate.isPlayback)
		{
		    if(playbackInput(&memory, &rstate, &input))
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
		updateGame(&memory, &input);
		
		KeyboardState* ktemp = input.currentKeyboard;
		input.currentKeyboard = input.previousKeyboard;
		input.previousKeyboard = ktemp;

		memcpy(input.currentKeyboard->keys, input.previousKeyboard->keys, sizeof(bool32) * VOIDSTORM_NUM_KEYS);
		
		ControllerState* temp = input.currentController;
		input.currentController = input.previousController;
		input.previousController = temp;

		frameCounter++;
	    
	    }

	    endRecordingState(&rstate);

	    shutdownGame(&memory);

	    releaseMemory(&memory);
	}
	else
	{
	    printf("ERROR: Failed to create window\n");
	    return 1;
	}
    }
    else
    {
	printf("ERROR: Failed to create window class\n");
	return 1;
    }

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
compareFileTime(FileTime *a, FileTime *b)
{
    return CompareFileTime(&a->time, &b->time);
}

File
readEntireFile(const char *filename)
{
    File result = {};
    
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(file != INVALID_HANDLE_VALUE)
    {
	LARGE_INTEGER size;
	if(GetFileSizeEx(file, &size))
	{
	    assert(size.QuadPart < 0xFFFFFFFF);
	    DWORD size32 = (DWORD)size.QuadPart;
	    
	    result.data = (uint8_t *)VirtualAlloc(0, size32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	    if(result.data)
	    {
		DWORD read;
		if(ReadFile(file, result.data, size32, &read, 0))
		{
		    result.size = (size_t)size32;
		}
	    }
	}
	
	CloseHandle(file);
    }

    return result;
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
    // TODO (daniel): Add InterlockedCompareExchange so any thread can add
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


uint64_t
getPerformanceCounter()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return (uint64_t)result.QuadPart;
}

uint64_t
getPerformanceFrequency()
{
    // TODO (daniel): This value dosen't change during runtime right?
    // So we could store it as a global and return it here whenever its needed
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return (uint64_t)result.QuadPart;
}
