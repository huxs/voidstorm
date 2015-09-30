#pragma once

#include "dcutil_helper.h"

#include <stdint.h>
#include <stdlib.h>

namespace dcutil
{
    DCUTIL_API uint32_t fnv32(const char* phrase);
    DCUTIL_API uint32_t murmur3_32(const char *key, uint32_t len);

    inline uint32_t sdbm32(const char* str, size_t len)
    {
	// Implementation of sdbm a public domain string hash from Ozan Yigit
	// see: http://www.eecs.harvard.edu/margo/papers/usenix91/paper.ps

	uint32_t hash = 0;
	typedef const char* pointer;
	for (pointer it = str, end = str + len; it != end; ++it)
	    hash = *it + (hash << 6) + (hash << 16) - hash;

	return hash;
    }    
}
