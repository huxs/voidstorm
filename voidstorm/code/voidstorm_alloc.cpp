
void* LuaAllocatorCallback(void *ud, void *ptr, size_t osize, size_t nsize)
{
    LuaAllocatorUserData *data = (LuaAllocatorUserData *)ud;
    dcutil::PoolAllocator *pool = data->pool;
    HeapAllocator *heap = data->heap;
    
    void* pRet = NULL;
    if(nsize == 0)
    {
	if (osize < VOIDSTORM_SCRIPT_ELEMENT_SIZE)
	{
	    pool->free(ptr);
	}
	else
	{
	    heap->free(ptr);
	    
	}
	
	return 0;	
	//PRINT("DeAllocating %p - Size: %d\n", ptr, (int)osize);
    }
    else
    {
	if (nsize < VOIDSTORM_SCRIPT_ELEMENT_SIZE)
	{
	    pRet = pool->alloc(0);
	    if(ptr)
	    {
		if(osize > 0 && osize < VOIDSTORM_SCRIPT_ELEMENT_SIZE)
		{
		    memcpy(pRet, ptr, VOIDSTORM_SCRIPT_ELEMENT_SIZE);
		    pool->free(ptr);
		}
		else if(osize > VOIDSTORM_SCRIPT_ELEMENT_SIZE)
		{
		    memcpy(pRet, ptr, VOIDSTORM_SCRIPT_ELEMENT_SIZE);
		    heap->free(ptr);
		}
	    }
	}
	
	else
	{
	    if (osize > 0 && osize < VOIDSTORM_SCRIPT_ELEMENT_SIZE)
	    {
		pRet = heap->realloc(NULL, nsize);
		memcpy(pRet, ptr, VOIDSTORM_SCRIPT_ELEMENT_SIZE);
		pool->free(ptr);
	    }
	    else
	    {	
		pRet = heap->realloc(ptr, nsize);
	    }
	}	
	//PRINT("Allocating %p - Size: %d\n", pRet, (int)nsize);
    }

    return pRet;
}

extern HeapAllocator* g_heapAllocator;

void* TinyStlAllocator::static_allocate(size_t _bytes)
{
    return g_heapAllocator->alloc(_bytes);
}
void TinyStlAllocator::static_deallocate(void* _ptr, size_t)
{
    g_heapAllocator->free(_ptr);
}

		
		
