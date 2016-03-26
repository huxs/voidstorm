#pragma once

#include <SDL2/SDL.h>

#include <dcfx/context.h>
#include <dcutil/macros.h>

#include <stdint.h>
typedef int32_t bool32;

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "voidstorm_alloc.h"
#include "voidstorm_input.h"

struct WorkQueue;
struct WorkThreadContext
{
    WorkQueue* queue;
    void* data;
};

#define WORK_QUEUE_CALLBACK(name) void name(WorkQueue* queue, void* data, WorkThreadContext* context)
typedef WORK_QUEUE_CALLBACK(WorkQueueCallback);

#ifdef WIN32
#include "windows/voidstorm_win32.h"
#define strcpy strcpy_s
#define strcat strcat_s
#define sprintf sprintf_s
#endif

#ifdef VOIDSTORM_INTERNAL
#define TIME_BLOCK(id) TimedBlock id(__COUNTER__, #id);

struct ProfileRecord
{
    char* name;
    double time;
};

extern ProfileRecord g_counters[];

// TODO (daniel): Make hit count, add __rdtsc to count clock cycles
// and make thread safe by atomic operations.
struct TimedBlock
{
    TimedBlock(int counter, char *name)
	{
	    record = &g_counters[counter];
	    record->name = name;
	    count = SDL_GetPerformanceCounter();
	}

    ~TimedBlock()
	{
	    uint64_t end = SDL_GetPerformanceCounter();
	    record->time = ((1000.0*(end - count)) / (double)SDL_GetPerformanceFrequency());
	}

    uint64_t count;
    ProfileRecord *record;
};

#else
#define TIME_BLOCK(id)
#endif

struct GameMemory
{
    void *permanentStoragePtr;
    size_t permanentStorageSize;
    void *transientStoragePtr;
    size_t transientStorageSize;

    HeapAllocator *heapAllocator;
    dcutil::StackAllocator *permStackAllocator;
    dcutil::StackAllocator *gameStackAllocator;

    // TODO (daniel): Rewrite the lua allocator to use a HeapAllocator we do not need to store an mspace
    mspace ms;
};

// Callback functions
bool initializeGame(GameMemory *memory, dcfx::Context *renderContext);
int updateGame(GameMemory *memory, GameInput *input);
int shutdownGame(GameMemory *memory);
// TODO (daniel): void resizeWindow(uint32_t w, uint32_t h);
// TODO (daniel): void playbackCompleted(); 

// File operations
bool getLastWriteTime(char *filename, FileTime *filetime);
int compareFileTime(FileTime& a, FileTime& b);
// TODO (daniel): void readFile();

// Threadpool operations
void setupWorkQueue(WorkQueue* queue, uint32_t threadCount, WorkThreadContext *contexts);
void addEntry(WorkQueue *queue, WorkQueueCallback *callback, void *data);
bool doNextEntry(WorkQueue *queue, WorkThreadContext *context);

// Timing operations needs to be in the platform API
