#pragma once

// NOTE: Padding issues on android
// XXX0 YY00 ZZZZ
// XXXX YYY0 ZZ00

#ifdef ANDROID
struct SpriteVertex
{
    glm::vec4 color;
    glm::vec3 position;
    glm::vec2 texcoord;
};
#else
struct SpriteVertex
{
    glm::vec3 position;
    glm::vec2 texcoord;
    glm::vec4 color;
};
#endif

// NOTE (daniel): Mabey convert this into SOA?
struct SpriteInfo
{	
    glm::vec4 destination;
    glm::vec4 source;
    glm::vec4 color;
    glm::vec4 originRotationDepth;

    dcfx::TextureHandle texture;
};

struct SpriteFont
{
    struct Glyph
    {
	uint32_t character;
	uint32_t left;
	uint32_t top;
	uint32_t right;
	uint32_t bottom;
	float offsetX;
	float offsetY;
	float advanceX;
    };

    struct TextureDesc
    {
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t stride;
	uint32_t rows;
    } desc;

    tinystl::vector<Glyph> glyphs;
    dcfx::TextureHandle texture;
};

class SpriteBatch
{
public:
    SpriteBatch(dcfx::Context* renderCtx);
    ~SpriteBatch();

    void write(const char* text, const glm::vec2& position);

    void flush(int view);

    void setBlendState(uint32_t blendState);
    void setSamplerState(uint32_t samplerState);
    void setCustomProgram(dcfx::ProgramHandle customProgram);
    void setTexture(dcfx::TextureHandle texture);
    void setDestination(const glm::vec4& destination);
    void setSource(const glm::vec4& source);
    void setColor(const glm::vec4& color);
    void setOrigin(const glm::vec2& origin);
    void setRotation(float rotation);
    void setDepth(float depth);

    void submit();
    void reset();
    
private:
    void setRenderStates(int view);
    void flushBatch(int view);
    void renderBatch(int view, dcfx::TextureHandle texture, SpriteInfo** sprites, int count);
    void bufferSprite(SpriteInfo* sprite, SpriteVertex* data, int index);
    void sortSprites();
	
    static const int MaxSprites = (1 << 18); // Number of sprites 
    static const int QueueSize = MaxSprites;
    static const int VerticesPerSprite = 4;
    static const int IndicesPerSprite = 6;

    dcfx::Context* renderCtx;
    dcfx::UniformHandle colorSampler;
    dcfx::VertexDecl vertexDecl;
    dcfx::BufferHandle vertexBuffer;
    dcfx::BufferHandle indexBuffer;
    dcfx::ProgramHandle selectedProgram;
    dcfx::ProgramHandle defaultProgram;
    dcfx::ProgramHandle customProgram;

    SpriteInfo spriteQueue[QueueSize];	
    uint32_t spriteQueueCount;
    SpriteInfo* spriteSortList[QueueSize];	
    uint32_t spriteCount;
	
    uint32_t blendState;
    uint32_t samplerState;

    // NOTE (daniel): Later we probably want multiple fonts
    SpriteFont defaultFont;
};

inline void SpriteBatch::setBlendState(uint32_t blendState)
{
    this->blendState = blendState;
}

inline void SpriteBatch::setSamplerState(uint32_t samplerState)
{
    this->samplerState = samplerState;
}

inline void SpriteBatch::setCustomProgram(dcfx::ProgramHandle customProgram)
{
    this->customProgram = customProgram;
}

inline void SpriteBatch::setTexture(dcfx::TextureHandle texture)
{
    if(texture.index == dcfx::InvalidHandle)
    {
	PRINT("Invalid texture %d\n", texture.index);
    }

    spriteQueue[spriteQueueCount].texture = texture;
}

inline void SpriteBatch::setDestination(const glm::vec4& destination)
{
    spriteQueue[spriteQueueCount].destination = destination;
}

inline void SpriteBatch::setSource(const glm::vec4& source)
{
    spriteQueue[spriteQueueCount].source = source;
}

inline void SpriteBatch::setColor(const glm::vec4& color)
{
    spriteQueue[spriteQueueCount].color = color;
}

inline void SpriteBatch::setOrigin(const glm::vec2& origin)
{
    spriteQueue[spriteQueueCount].originRotationDepth.x = origin.x;
    spriteQueue[spriteQueueCount].originRotationDepth.y = origin.y;
}

inline void SpriteBatch::setRotation(float rotation)
{
    spriteQueue[spriteQueueCount].originRotationDepth.z = rotation;
}

inline void SpriteBatch::setDepth(float depth)
{
    spriteQueue[spriteQueueCount].originRotationDepth.w = depth;
}
