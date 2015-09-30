#pragma once

enum SpriteSortMode 
{
    TEXTURE,
    BACKTOFRONT,
    FRONTTOBACK
};

// TODO:
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

// NOTE: Mabey convert this into SOA?
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

    void draw(int view);

    inline void setSortMode(SpriteSortMode sortMode);
    inline void setBlendState(uint32_t blendState);
    inline void setCustomProgram(dcfx::ProgramHandle customProgram);
    inline void setTexture(dcfx::TextureHandle texture);
    inline void setDestination(const glm::vec4& destination);
    inline void setSource(const glm::vec4& source);
    inline void setColor(const glm::vec4& color);
    inline void setOrigin(const glm::vec2& origin);
    inline void setRotation(float rotation);
    inline void setDepth(float depth);

    void submit();
    void reset();
    
private:
    void setRenderStates(int view);
    void flushBatch(int view);
    void renderBatch(int view, dcfx::TextureHandle texture, SpriteInfo** sprites, int count);
    void bufferSprite(int vertices, SpriteInfo* sprite, SpriteVertex* data, int index);
    void sortSprites();
	
    static const int BatchSize = 32768;
    static const int QueueSize = 4096;
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
    int VBOPosition;
	
    SpriteSortMode sortMode;
    uint32_t blendState;

    // TODO: Later we probely want to support users passing fonts.
    SpriteFont defaultFont;
};

inline void SpriteBatch::setSortMode(SpriteSortMode sortMode)
{
    this->sortMode = sortMode;
}

inline void SpriteBatch::setBlendState(uint32_t blendState)
{
    this->blendState = blendState;
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
