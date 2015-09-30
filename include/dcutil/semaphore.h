#pragma once

#include "dcutil_helper.h"

#include <mutex>
#include <condition_variable>

namespace dcutil
{ 
    class Semaphore
    {
    public:
	DCUTIL_API Semaphore();
	DCUTIL_API Semaphore(int32_t count);
	DCUTIL_API void notify();
	DCUTIL_API void wait();
    private:
	int32_t m_count;
	std::mutex m_mutex;
	std::condition_variable m_cv;
    };
}
