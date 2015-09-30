#include "../voidstorm_platform.h"

#define PERMANENT_STORAGE_SIZE Megabytes(256)
#define TRANSIENT_STORAGE_SIZE Megabytes(128)

static void printGetLastError()
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

bool getLastWriteTime(char* filename, FileTime* filetime)
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

int compareFileTime(FileTime& a, FileTime& b)
{
    return CompareFileTime(&a.time, &b.time);
}

bool initializeMemory(GameMemory* memory)
{
#ifdef _HAVE_X64
    INT_PTR BaseAddress = 0x00000000003d0000;
#else
	INT_PTR BaseAddress = 0;
#endif
    /*
    memory->permanentStorageSize = VOIDSTORM_APPLICATION_HEAP
	+ VOIDSTORM_RENDER_HEAP
	+ VOIDSTORM_APPLICATION_PERMANENT_STACK;
    */

    memory->permanentStorageSize = PERMANENT_STORAGE_SIZE;
    memory->transientStorageSize = TRANSIENT_STORAGE_SIZE;

    size_t totalSize = memory->permanentStorageSize + memory->transientStorageSize;
	
    memory->permanentStorage = VirtualAlloc((LPVOID)BaseAddress, totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    if(memory->permanentStorage == NULL)
	{
		printf("Failed to VirtuaAlloc.\n");
		printGetLastError();
		return false;
	}
	

    memory->transientStorage = (uint8_t*)memory->permanentStorage + memory->permanentStorageSize;

    return true;
}

bool32 releaseMemory(GameMemory* memory)
{
    bool32 success = VirtualFree(
	memory->permanentStorage,      
	0,             
	MEM_RELEASE);  

    return success; 
}



bool setupRecordingState(GameMemory* memory, RecordingState* state)
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

void endRecordingState(RecordingState* state)
{
    CloseHandle(state->handle);
}

void beginRecord(GameMemory* memory, RecordingState* state)
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
    
    CopyMemory(state->memory, memory->permanentStorage, memory->permanentStorageSize);
}

void endRecord(RecordingState* state)
{
    if(!state->isRecording)
	return;
    
    state->isRecording = false;
    
    CloseHandle(state->recordHandle);
    printf("End recording.\n");
}

void beginPlayback(GameMemory* memory, RecordingState* state)
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
    
    CopyMemory(memory->permanentStorage, state->memory, memory->permanentStorageSize);
}

void endPlayback(RecordingState* state)
{
    if(!state->isPlayback)
	return;
    
    state->isPlayback = false;
    
    CloseHandle(state->playbackHandle);
    printf("End playback\n");
}

void recordInput(RecordingState* state, GameInput* input)
{
    DWORD bytesWritten;
    WriteFile(state->recordHandle, input, sizeof(*input), &bytesWritten, 0);
}

bool playbackInput(GameMemory* memory, RecordingState* state, GameInput* input)
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
