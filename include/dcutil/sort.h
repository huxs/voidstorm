#pragma once

#include <stdint.h>
#include <TINYSTL/vector.h>

#define RADIXSORT_BASE 10

namespace dcutil
{
    inline uint64_t getMax(uint64_t arr[], size_t n)
    {
	uint64_t mx = arr[0];
	for (size_t i = 1; i < n; i++)
	    if (arr[i] > mx)
		mx = arr[i];
	return mx;
    }

    template<class T>
    void radixSort64(uint64_t* keys, T* values, size_t num)
    {
	tinystl::vector<uint64_t> buckets[RADIXSORT_BASE];
	tinystl::vector<T> bucketsValues[RADIXSORT_BASE];

	uint64_t max = getMax(keys, num);

	for (uint64_t radix = 1; max > radix; radix *= RADIXSORT_BASE)
	{
	    for (size_t n = 0; n < num; n++)
	    {
		int b = (keys[n] / radix) % RADIXSORT_BASE;
	
		buckets[b].push_back(keys[n]);
		bucketsValues[b].push_back(values[n]);
	    }

	    int r, k;
	    for (r = k = 0; r < RADIXSORT_BASE; r++)
	    {
		for (size_t v = 0; v < buckets[r].size(); v++)
		{
		    keys[k] = buckets[r][v];
		    values[k] = bucketsValues[r][v];
		    k++;
		}

		buckets[r].clear();
		bucketsValues[r].clear();
	    }
	}
    }
}
