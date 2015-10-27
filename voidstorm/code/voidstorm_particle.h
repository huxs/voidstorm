#pragma once

// Effect handle?
struct ParticleHandle { uint16_t index; };

// Effect system?
class ParticleEngine
{
public:
    ParticleEngine(dcutil::StackAllocator* stack, dcfx::Context* renderCtx, SpriteBatch* sb);

    void reset();
    
    ParticleHandle createHandle(ParticleEffectDescription* desc);
    void deleteHandle(ParticleHandle handle);

    void play(ParticleHandle handle);
    void stop(ParticleHandle handle);
    
    void setPosition(ParticleHandle handle, const glm::vec2& position);
    void setRotation(ParticleHandle handle, float rotation);
    void setDepth(ParticleHandle handle, float depth);
    
    void update(float dt);
    void render(int view);

private:

    static const int MaxEffects = 1000;
    static const int MaxEmittersPerEffect = 128;
    static const int MaxEmitters = 5000;

    struct Particle
    {
	glm::vec2 position;
	glm::vec2 acceleration;
	glm::vec2 velocity;
	glm::vec4 color;
	glm::vec2 size;
	float rotation;
	float time;
	float lifetime;
	Texture* texture;
    };

    enum class ParticleBufferSizeTier
    {
	LOW,
	MEDIUM,
	HIGH
    };
    
    struct ParticleBuffer
    {
	ParticleBuffer() : particles(nullptr) {}
    
	Particle* particles;
	int size;
	int used;
    };

    enum class EmitterState
    {
	EMITTING, // Emitter is running
	ENDING, // Emittter has stoped, but simulate remaining particles
	FINNISHED // All particles in the emitter has passed it's lifetime
    };
    
    struct ParticleEmitter
    {
	ParticleEmitterDescription* emitter;
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
    int emittersInPlayCount;

    dcutil::PoolAllocator memoryPools[3];
};
