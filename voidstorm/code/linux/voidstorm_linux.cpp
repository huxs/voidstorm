#include "../voidstorm_platform.h"

#include <sys/mman.h>

#define PERMANENT_STORAGE_SIZE Megabytes(64)
#define TRANSIENT_STORAGE_SIZE Megabytes(0)

bool getLastWriteTime(char* filename, FileTime* filetime)
{
/*    WIN32_FIND_DATA findData;
    HANDLE handle = FindFirstFileA(filename, &findData);
    if(handle != INVALID_HANDLE_VALUE)
    {
	filetime->time = findData.ftLastWriteTime;
	FindClose(handle);
	return true;
    }*/
    
    return false;
}

int compareFileTime(FileTime& a, FileTime& b)
{
    //return CompareFileTime(&a.time, &b.time);
    return 0;
}

bool initializeMemory(GameMemory* memory)
{
    //LPVOID BaseAddress = (LPVOID)0x000000d0ca3d0000;

    memory->permanentStorageSize = PERMANENT_STORAGE_SIZE;
    memory->transientStorageSize = TRANSIENT_STORAGE_SIZE;

    size_t totalSize = memory->permanentStorageSize + memory->transientStorageSize;
	
    //memory->permanetStorage = VirtualAlloc(BaseAddress, totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    /*memory->permanentStorage = mmap(0, totalSize,
				    PROT_READ | PROT_WRITE,
				    MAP_ANON | MAP_PRIVATE,
				    -1, 0);*/

    memory->permanentStorage = new char[totalSize];
    
    if(memory->permanentStorage == NULL)
	return false;
    
    memory->transientStorage = (uint8_t*)memory->permanentStorage + memory->permanentStorageSize;

    return true;
}

bool32 releaseMemory(GameMemory* memory)
{
    delete[] (uint8_t*)memory->permanentStorage;
}

bool setupRecordingState(GameMemory* memory, RecordingState* state)
{
    state->isRecording = false;
    state->isPlayback = false;
    return false;
}

void endRecordingState(RecordingState* state)
{
}

void beginRecord(GameMemory* memory, RecordingState* state)
{
}

void endRecord(RecordingState* state)
{
}

void beginPlayback(GameMemory* memory, RecordingState* state)
{
}

void endPlayback(RecordingState* state)
{
}

void recordInput(RecordingState* state, GameInput* input)
{
}

bool playbackInput(GameMemory* memory, RecordingState* state, GameInput* input)
{
}


