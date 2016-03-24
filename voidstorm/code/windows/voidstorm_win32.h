#pragma once

#include <windows.h>

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

struct WorkQueueEntry
{
    WorkQueueCallback* callback;
    void* data;
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
