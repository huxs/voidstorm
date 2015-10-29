#pragma once

struct ParticleEffectHandle { uint16_t index; };

static const uint32_t s_particleBufferSize[] = 
{
    128,
    1024,
    2048
};
    
class ParticleEngine
{
public:
    ParticleEngine(dcutil::StackAllocator* stack, dcfx::Context* renderCtx, SpriteBatch* sb);

    void reset();
    
    ParticleEffectHandle createHandle(ParticleEffectDescription* desc);
    void deleteHandle(ParticleEffectHandle handle);

    void play(ParticleEffectHandle handle, uint32_t frames);
    void stop(ParticleEffectHandle handle);
    
    void setPosition(ParticleEffectHandle handle, const glm::vec2& position);
    void setRotation(ParticleEffectHandle handle, float rotation);
    void setDepth(ParticleEffectHandle handle, float depth);
    
    void update(float dt);
    void render(int view);
    void debug(int view);

private:

    static const int MaxEffects = 1000;
    static const int MaxEmittersPerEffect = 1000;
    static const int MaxEmitters = 5000;

    struct Particle
    {
	glm::vec2 position;
	glm::vec2 acceleration;
	glm::vec2 velocity;
	glm::vec4 color;
	glm::vec2 size;
        float depth;
	float rotation;
	float time;
	float lifetime;
	Texture* texture;
    };

    enum ParticleBufferSizeTier
    {
	LOW,
	MEDIUM,
	HIGH,
    };

    struct ParticleBuffer
    {
	ParticleBuffer() : particles(nullptr) {}
    
	Particle* particles;
	uint32_t size;
	uint32_t used;
    };

    enum class EmitterState
    {
	EMITTING, // Emitter is running
	ENDING, // Emittter has stoped, but simulate remaining particles
	FINNISHED // All particles in the emitter has passed it's lifetime
    };

    /* TODO (daniel): Remove the state from the struct and create multiple
       venues for the emitter to live instead */
    
    struct ParticleEmitter
    {
	ParticleEmitterDescription* desc;
	glm::vec2 position;
	float rotation;
	float depth;
	float lifetime;
	float time;
	EmitterState state;
	ParticleBuffer b0, b1;
	ParticleBuffer* active;
	ParticleBuffer* second;
    };

    struct EmitterHandle { uint16_t index; };
    
    struct ParticleEffect
    {
	ParticleEffect() : rotation(0.0f), depth(1.0f) {}
    
	ParticleEffectDescription* desc;
	glm::vec2 position;
	float rotation;
	float depth;
	EmitterHandle emitters[ParticleEngine::MaxEmittersPerEffect];
    };

    void prepass();
    void spawn(ParticleEmitter* emitter);
    void emit(float dt);
    void simulate(float dt);
    void swap();

    dcutil::StackAllocator* stack;
    dcfx::Context* renderCtx;
    SpriteBatch* sb;

    dcutil::HandleAllocator<MaxEffects> effectHandles;
    ParticleEffect effects[MaxEffects];
    ParticleEffect* effectsInPlay[MaxEffects];

    dcutil::HandleAllocator<MaxEmitters> emitterHandles;
    ParticleEmitter emitters[MaxEmitters];
    ParticleEmitter* emittersInPlay[MaxEmitters];
    uint32_t emittersInPlayCount;

    dcutil::PoolAllocator memoryPools[3];
};
