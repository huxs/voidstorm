#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef int32_t bool32;

#include "voidstorm_alloc.h"
#include "voidstorm_input.h"

#include <dcutil/macros.h>

/*
  Game callbacks
  
*/

struct GameMemory
{
    void *permanentStoragePtr;
    size_t permanentStorageSize;
    void *transientStoragePtr;
    size_t transientStorageSize;

    HeapAllocator *renderHeapAllocator;
    HeapAllocator *globalHeapAllocator;

    dcutil::StackAllocator *permStackAllocator;
    dcutil::StackAllocator *gameStackAllocator;
};

bool initializeGame(GameMemory *memory, void *hwnd);
int updateGame(GameMemory *memory, GameInput *input);
int shutdownGame(GameMemory *memory);
// TODO (daniel): void resizeWindow(uint32_t w, uint32_t h);
// TODO (daniel): void playbackCompleted(); 

/*
  File handling
  
*/

struct File
{
    uint8_t *data;
    size_t size;
};

struct FileTime;
bool getLastWriteTime(char *filename, FileTime *filetime);
int compareFileTime(FileTime *a, FileTime *b);
File readEntireFile(const char *filename);

/*
  Threading
  
*/

struct WorkQueue;
struct WorkThreadContext
{
    WorkQueue *queue;
    void *data; // User data for a worker thread
};

#define WORK_QUEUE_CALLBACK(name) void name(WorkQueue *queue, void *data, WorkThreadContext *context)
typedef WORK_QUEUE_CALLBACK(WorkQueueCallback);

struct WorkQueueEntry
{
    WorkQueueCallback *callback;
    void* data;
};

void setupWorkQueue(WorkQueue* queue, uint32_t threadCount, WorkThreadContext *contexts);
void addEntry(WorkQueue *queue, WorkQueueCallback *callback, void *data);
bool doNextEntry(WorkQueue *queue, WorkThreadContext *context);

/*
  Timing and Profiling
  
*/

uint64_t getPerformanceCounter();
uint64_t getPerformanceFrequency();

#ifdef VOIDSTORM_INTERNAL
#define TIME_BLOCK(id) TimedBlock id(__COUNTER__, #id);

struct ProfileRecord
{
    char* name;
    double time;
};

extern ProfileRecord g_counters[];

// TODO (daniel): Make hit count, add __rdtsc to count clock cycles
// and make thread safe by atomic operations
struct TimedBlock
{
    TimedBlock(int counter, char *name)
	{
	    record = &g_counters[counter];
	    record->name = name;
	    count = getPerformanceCounter();
	}

    ~TimedBlock()
	{
	    uint64_t end = getPerformanceCounter();
	    record->time = ((1000.0*(end - count)) / (double)getPerformanceFrequency());
	}

    uint64_t count;
    ProfileRecord *record;
};

#else
#define TIME_BLOCK(id)
#endif

#ifdef WIN32 
#include "windows/voidstorm_win32.h"
#endif
