#pragma once

#include <windows.h>

#define strcpy strcpy_s
#define strcat strcat_s
#define sprintf sprintf_s

#define GAME_STORAGE_RESERVED_SIZE Megabytes(1)
#define PERMANENT_STORAGE_SIZE Megabytes(256)
#define TRANSIENT_STORAGE_SIZE Megabytes(128)

struct FileTime
{
    FILETIME time;
};

struct RecordingState
{
    void* memory;
    bool isRecording;
    bool isPlayback;
    HANDLE handle;
    HANDLE recordHandle;
    HANDLE playbackHandle;
    HANDLE memoryMap;
};

struct WorkQueue
{
    uint32_t volatile completionGoal;
    uint32_t volatile completionCount;
    uint32_t volatile nextEntryToWrite;
    uint32_t volatile nextEntryToRead;

    HANDLE semaphore;

    WorkQueueEntry entries[256];
};

