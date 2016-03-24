#pragma once

#include <SDL2/SDL.h>
#include <dcutil/macros.h>

#include <stdint.h>
typedef int32_t bool32;

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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

#include "voidstorm_input.h" // GameInput

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
    void* permanentStorage;
    size_t permanentStorageSize;
    void* transientStorage;
    size_t transientStorageSize;
};

bool getLastWriteTime(char *filename, FileTime *filetime);
int compareFileTime(FileTime& a, FileTime& b);

bool initializeMemory(GameMemory *memory);
bool32 releaseMemory(GameMemory *memory);

bool setupRecordingState(GameMemory *memory, RecordingState *state);
void endRecordingState(RecordingState *state);

void beginRecord(GameMemory *memory, RecordingState *state);
void endRecord(RecordingState *state);

void beginPlayback(GameMemory *memory, RecordingState *state);
void endPlayback(RecordingState *state);

void recordInput(RecordingState* state, GameInput *input);
bool playbackInput(GameMemory *memory, RecordingState *state, GameInput* input);

void setupWorkQueue(WorkQueue* queue, uint32_t threadCount, WorkThreadContext *contexts);

void addEntry(WorkQueue *queue, WorkQueueCallback *callback, void *data);
bool doNextEntry(WorkQueue *queue, WorkThreadContext *context);
//void completeAllWork(WorkQueue* queue);
