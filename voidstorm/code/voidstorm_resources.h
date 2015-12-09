#pragma once

template<class T>
struct ResourceNode
{
    ResourceNode() : hash(-1), resource(nullptr), next(nullptr) { }

    int hash;
    T* resource;
    ResourceNode* next;
};

template<class T>
struct ResourceMap
{
    ResourceMap(dcutil::StackAllocator* _stack) : stack(_stack) {}
				       
    T* add(const char* name);
    T* remove(T* resource);
    T* lookup(const char* name);

    ResourceNode<T> map[100];
    dcutil::StackAllocator* stack;
};

template<class T>
T* ResourceMap<T>::add(const char* name)
{
    T* result = nullptr;
    
    int hash = dcutil::sdbm32(name, strlen(name));
    int slot = hash & (ARRAYSIZE(map)-1);
    
    ResourceNode<T>* node = map + slot;
    do
    {
	if(node->hash == hash) // If resource already exists
	{
	    result = node->resource;
	    break;
	}
	
	if(node->resource && !node->next) // Node exist but next does not, extend the internal chain
	{
	    node->next = (ResourceNode<T>*)stack->alloc(sizeof(ResourceNode<T>));

	    node = node->next;
	    node->hash = -1;
	    node->resource = nullptr;
	    node->next = nullptr;
	}

	if(node->resource == nullptr) // Found a empty node, store the resource
	{
	    result = (T*)stack->alloc(sizeof(T));
	    node->resource = result;
	    node->hash = hash;
	    break;
	}

	node = node->next;
	
    } while(node);

    return result;
}

template<class T>
T* ResourceMap<T>::remove(T* resource)
{
    T* result = nullptr;
    
    for(int i = 0; i < ARRAYSIZE(map); ++i)
    {
	ResourceNode<T>* node = map + i;
	if(node->resource == resource)
	{
	    result = node->resource;
	    node->resource = nullptr;
	    break;
	}
    }

    return result;
}

template<class T>
T* ResourceMap<T>::lookup(const char* name)
{
    T* result = nullptr;

    int hash = dcutil::sdbm32(name, strlen(name));
    int slot = hash & (ARRAYSIZE(map)-1);

    ResourceNode<T>* node = map + slot;
    do
    {
	if(node->hash == hash)
	{
	    result = node->resource;
	    break;
	}

	node = node->next;
	
    } while(node);

    return result;
}

struct Texture
{
    dcfx::TextureHandle handle;
    uint32_t width;
    uint32_t height;
};

class TextureManager
{
public:
    TextureManager(dcfx::Context* _renderCtx, dcutil::StackAllocator* _stack)
	    : renderCtx(_renderCtx), textures(_stack) {}

    Texture* load(const char* name);
    void remove(Texture* texture);
    void clean();
    
private:
    dcfx::Context* renderCtx;
    ResourceMap<Texture> textures;
};

struct ParticleEmitterDescription
{
    int bufferSizeTier;
    int particlesPerEmit;
    float spawnTime;
    float lifetime;
    float lifetimeMin;
    float lifetimeMax;
    float sizeMin;
    float sizeMax;
    float rotationMin;
    float rotationMax;
    float depthMin;
    float depthMax;
    glm::vec4 colorMin;
    glm::vec4 colorMax;
    glm::vec4 startColor;
    glm::vec4 endColor;
    glm::vec2 force;
    float spread;
    glm::vec2 position;
    bool32 relative;
    Texture* texture;
    ParticleEmitterDescription* next;
};

struct ParticleEffectDescription
{
    ParticleEffectDescription() : emitter(nullptr), count(0) {}
    
    uint32_t count;
    ParticleEmitterDescription* emitter;
};

class ParticleEffectManager
{
public:
    ParticleEffectManager(dcutil::StackAllocator* _stack)
	    : stack(_stack), effects(_stack) {}

    // NOTE: Loads a particle effect description off the lua stack
    ParticleEffectDescription* load(lua_State* luaState);
    
    void remove(ParticleEffectDescription* effect);
    void clean();

private:
    dcutil::StackAllocator* stack;
    ResourceMap<ParticleEffectDescription> effects;
};

struct ResourceManager
{
    ResourceManager(dcutil::StackAllocator* stack, dcfx::Context* renderCtx)
	    : textures(renderCtx, stack), effects(stack)
	{}
    ~ResourceManager();

    TextureManager textures;
    ParticleEffectManager effects;
};
