struct LuaAllocatorUserData
{
    dcutil::Pool* pool;
    mspace ms;
};

void* LuaAllocatorCallback(void* ud, void* ptr, size_t osize, size_t nsize)
{
    LuaAllocatorUserData* data = (LuaAllocatorUserData*)ud;
    dcutil::Pool* pool = data->pool;
    
    void* pRet = NULL;
    if(nsize == 0)
    {
	if (osize < VOIDSTORM_SCRIPT_ELEMENT_SIZE)
	{
	    pool->free(ptr);
	}
	else
	{
	    mspace_free(data->ms, ptr);    
	}
	
	return 0;	
	//PRINT("DeAllocating %p - Size: %d\n", ptr, (int)osize);
    }
    else
    {
	if (nsize < VOIDSTORM_SCRIPT_ELEMENT_SIZE)
	{
	    pRet = pool->alloc();
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
		    mspace_free(data->ms, ptr);
		}
	    }
	}
	
	else
	{
	    if (osize > 0 && osize < VOIDSTORM_SCRIPT_ELEMENT_SIZE)
	    {
		pRet = mspace_realloc(data->ms, NULL, nsize);
		memcpy(pRet, ptr, VOIDSTORM_SCRIPT_ELEMENT_SIZE);
		pool->free(ptr);
	    }
	    else
	    {	
		pRet = mspace_realloc(data->ms, ptr, nsize);
	    }
	}	
	//PRINT("Allocating %p - Size: %d\n", pRet, (int)nsize);
    }

    return pRet;
}

extern HeapAllocator* g_allocator;

void* TinyStlAllocator::static_allocate(size_t _bytes)
{
    return g_allocator->alloc(_bytes);
}
void TinyStlAllocator::static_deallocate(void* _ptr, size_t)
{
    g_allocator->free(_ptr);
}

		
		
