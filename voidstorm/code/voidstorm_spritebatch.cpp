#include <dcutil/sort.h>
#include <dcutil/memoryreader.h>

// TODO (daniel): Move fonts to resource manager
static SpriteFont loadFont(dcfx::Context* renderCtx, const char* filepath)
{
    SpriteFont font;

    tinystl::string assets(VOIDSTORM_FONT_DIRECTORY);
    tinystl::string file(filepath);
    assets.append(file.c_str(), file.c_str() + file.size());
    PRINT("%s\n", assets.c_str());  
    
    SDL_RWops* handle = SDL_RWFromFile(assets.c_str(), "rb");
    if (handle == nullptr)
    {
	PRINT("Failed to open file. %s\n", assets.c_str());
	return font;
    }

    // Get the size of the file in bytes
    SDL_RWseek(handle, 0, SEEK_END);
    size_t size = (unsigned int)SDL_RWtell(handle);
    SDL_RWseek(handle, 0, SEEK_SET);

    // Allocate space for storing the content of the file
    char* mem = (char*)renderCtx->frameAlloc(sizeof(char) * size);

    size_t bytes = SDL_RWread(handle, mem, size, 1);
    if (bytes != 1)
    {
	PRINT("Error reading from file %s.\n", filepath);
	return font;
    }
	
    dcutil::MemoryReader reader(mem, size);

    // Validate file header
    static const char valid[] = "DXTKfont";

    for(int i = 0; i < ARRAYSIZE(valid)-1; ++i)
    {
	char c;
	dcutil::read(&reader, c);
	if(c != valid[i])
	{
	    PRINT("File not valid DXTK font.\n");
	    return font;
	}
    }

    uint32_t glyphCount;
    dcutil::read(&reader, glyphCount);
    font.glyphs.resize(glyphCount);
    for(uint32_t i = 0; i < glyphCount; ++i)
    {
	dcutil::read(&reader, font.glyphs[i]);
    }
	
    float lineSpacing;
    dcutil::read(&reader, lineSpacing);

    char defaultChar[4];
    dcutil::read(&reader, defaultChar);

    dcutil::read(&reader, font.desc);

    size_t numPixels = font.desc.stride * font.desc.rows;
	
    uint8_t* pixels = (uint8_t*)renderCtx->frameAlloc(numPixels);
    reader.read(pixels, numPixels);

    font.texture = renderCtx->createTexture(font.desc.width, font.desc.height, dcfx::TextureFormat::RGBA8);
    renderCtx->updateTexture(font.texture, pixels);

    return font;
};

#define SAMPLER_STATE (DCFX_SAMPLER_MAG_LINEAR | DCFX_SAMPLER_MIN_LINEAR | DCFX_EDGE_FUNC(DCFX_SAMPLER_EDGE_CLAMP, DCFX_SAMPLER_EDGE_CLAMP, DCFX_SAMPLER_EDGE_CLAMP))  

SpriteBatch::SpriteBatch(dcfx::Context* _renderCtx)
	:
	renderCtx(_renderCtx),
	blendState(DCFX_DEFAULT_STATE),
	samplerState(SAMPLER_STATE),
	customProgram({dcfx::InvalidHandle}),
	spriteCount(0),
	spriteQueueCount(0)
{

    this->defaultFont = loadFont(renderCtx, "arial.spritefont");
    this->colorSampler = renderCtx->createUniform("g_colorMap", dcfx::UniformType::SAMPLER, 1);
	
    dcfx::ProgramDesc desc;
    desc.m_vert = loadShader(renderCtx, "sprite.vert", dcfx::ShaderType::VERTEX);
    desc.m_frag = loadShader(renderCtx, "sprite.frag", dcfx::ShaderType::FRAGMENT);
    defaultProgram = renderCtx->createProgram(desc);

    vertexDecl.begin();
    vertexDecl.add(0, 3, dcfx::AttributeType::FLOAT, false);
    vertexDecl.add(1, 2, dcfx::AttributeType::FLOAT, false);
    vertexDecl.add(2, 4, dcfx::AttributeType::FLOAT, false);
    vertexDecl.end();

    this->vertexBuffer = renderCtx->createVertexBuffer(
	nullptr,
	SpriteBatch::MaxSprites * SpriteBatch::VerticesPerSprite * sizeof(SpriteVertex),
	vertexDecl);

    uint32_t* indices = (uint32_t*)renderCtx->frameAlloc(
	SpriteBatch::MaxSprites * SpriteBatch::IndicesPerSprite * sizeof(uint32_t));

    // Counter-clockwise culling
    for(uint32_t vertex = 0, index = 0;
	vertex < SpriteBatch::MaxSprites * SpriteBatch::VerticesPerSprite;
	vertex += SpriteBatch::VerticesPerSprite, index += SpriteBatch::IndicesPerSprite)
    {
	indices[index] = vertex + 3;
	indices[index+1] = vertex + 2;
	indices[index+2] = vertex + 1;
	indices[index+3] = vertex + 3;
	indices[index+4] = vertex + 1;
	indices[index+5] = vertex;
    }
	
    this->indexBuffer = renderCtx->createBuffer(
	&indices[0],
	SpriteBatch::MaxSprites * SpriteBatch::IndicesPerSprite * sizeof(uint32_t),
	dcfx::BufferType::INDEX);
}

SpriteBatch::~SpriteBatch()
{
    renderCtx->deleteUniform(colorSampler);
    renderCtx->deleteBuffer(vertexBuffer);
    renderCtx->deleteBuffer(indexBuffer);
    renderCtx->deleteProgram(defaultProgram);
    renderCtx->deleteTexture(defaultFont.texture);
}

inline SpriteFont::Glyph* findGlyphInFont(char character, SpriteFont* font)
{
    for(SpriteFont::Glyph& g : font->glyphs) {
	if(g.character == character) {
	    return &g;
	}
    }
    return nullptr;
}

void SpriteBatch::write(const char* text, const glm::vec2& position)
{
    float x = 0;
    float y = 0;

    for(const char* c = text; *c; ++c)
    {
	SpriteFont::Glyph* glyph = findGlyphInFont(*c, &defaultFont);
	if(glyph != nullptr)
	{
	    x += glyph->offsetX;

	    int width = glyph->right - glyph->left;
	    int height = glyph->bottom - glyph->top;

	    glm::vec4 subrect = glm::vec4(
		(float)glyph->left / defaultFont.desc.width,
		(float)glyph->top / defaultFont.desc.height,
		(float)width / defaultFont.desc.width,
		(float)height / defaultFont.desc.height
		);

	    glm::vec2 offset = glm::vec2((x * -1) / width, (y + glyph->offsetY * -1)/height);

	    setDestination(glm::vec4(position.x, position.y, width, height));
	    setSource(subrect);
	    setOrigin(offset);
	    setTexture(defaultFont.texture);
	    setRotation(0);
	    setDepth(0);
	    setColor(glm::vec4(1,1,1,1));	

	    submit();
			
	    x += width + glyph->advanceX;
	}		
    }
}

void SpriteBatch::submit()
{
    if(spriteQueueCount >= QueueSize - 1)
    {
	//PRINT("SpriteQueue full %d\n", SpriteBatch::QueueSize);	
	return;
    }
	
    spriteQueueCount++;
}

void SpriteBatch::reset()
{
    spriteCount = 0;
    spriteQueueCount = 0;
}

void SpriteBatch::flush(int view)
{
    setRenderStates(view);
    flushBatch(view);
}

void SpriteBatch::setRenderStates(int view)
{
    // Set program
    if(customProgram.index != dcfx::InvalidHandle) {
	selectedProgram = customProgram;
    } 
    else {
	selectedProgram = defaultProgram;	
    }
    renderCtx->setProgram(selectedProgram);

    renderCtx->setState(blendState);
    renderCtx->setPrimitiveMode(DCFX_STATE_PT_TRIANGLES);
}

// Converts the sprites in the queue to batches
void SpriteBatch::flushBatch(int view)
{
    if(spriteQueueCount == 0) return;

    sortSprites();

    dcfx::TextureHandle batchTexture = {dcfx::InvalidHandle};
    uint32_t batchStart = 0;
    for(uint32_t pos = 0; pos < spriteQueueCount; pos++)
    {
	dcfx::TextureHandle spriteTexture = spriteSortList[pos]->texture;

	if(spriteTexture.index != batchTexture.index)
	{
	    if(pos > batchStart)
	    {
		//Flush
		renderBatch(
		    view,
		    batchTexture,
		    &spriteSortList[batchStart],
		    pos - batchStart);
	    }
	  
	    batchTexture = spriteTexture;
	    batchStart = pos;
	}
    }

    // Render final batch
    renderBatch(
	view,
	batchTexture,
	&spriteSortList[batchStart],
	spriteQueueCount - batchStart);

    // Reset the queue
    spriteQueueCount = 0;
}

void SpriteBatch::renderBatch(
    int view,
    dcfx::TextureHandle texture,
    SpriteInfo** sprites,
    int count)
{
    renderCtx->setTexture(colorSampler, texture, samplerState);
    
    // Batch size in bytes
    size_t size = count * SpriteBatch::VerticesPerSprite * sizeof(SpriteVertex);
    
    SpriteVertex* data = (SpriteVertex*)renderCtx->frameAlloc(size);

    // Convert sprite batch to vertex data
    for (int i = 0; i < count; i++)
    {
	bufferSprite(sprites[i], data, i);
    }

    // Current offset for the vertex buffer
    uint32_t offset = spriteCount * SpriteBatch::VerticesPerSprite * sizeof(SpriteVertex);
    
    assert((offset + size) <= MaxSprites * SpriteBatch::VerticesPerSprite * sizeof(SpriteVertex));
    
    renderCtx->updateBuffer(
	vertexBuffer,
        offset,
	size,
	&data[0]);
	
    renderCtx->setVertexBuffer(vertexBuffer, 0, 0);
    renderCtx->setIndexBuffer(
	indexBuffer,
	spriteCount * SpriteBatch::IndicesPerSprite,
	SpriteBatch::IndicesPerSprite * count);

    renderCtx->submit(view);
	
    spriteCount += count;
}

void SpriteBatch::bufferSprite(SpriteInfo* spriteInfo, SpriteVertex* data, int index)
{
    static const glm::vec4 cornerOffsets[SpriteBatch::VerticesPerSprite] =
	{
	    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
	    glm::vec4(1.0f, 0.0f, 1.0f, 0.0f),
	    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
 	};

    float rotation = spriteInfo->originRotationDepth.z;
	
    glm::vec4 row1(1,0,0,0);
    glm::vec4 row2(0,1,0,0);

    if(rotation != 0) {
	float _sin = sin(rotation);
	float _cos = cos(rotation);

	row1.x = _cos;
	row1.y = -_sin;
	row2.x = _sin;
	row2.y = _cos;
    }

    glm::vec2 origin = glm::vec2(spriteInfo->originRotationDepth.x, spriteInfo->originRotationDepth.y);

    // NOTE (daniel): This code is needed if we supply source in pixels.
    /*
      if(spriteInfo->source.z != 0 && spriteInfo->source.w != 0) {
      origin.x /= spriteInfo->source.z;
      origin.y /= spriteInfo->source.w;
      }*/

    for(int i = 0; i < SpriteBatch::VerticesPerSprite; i++)
    {
	glm::vec4 PosTex;
	PosTex = cornerOffsets[i];
		
	PosTex.x -= origin.x;
	PosTex.y -= origin.y;
		
	PosTex.x *= spriteInfo->destination.z;
	PosTex.y *= spriteInfo->destination.w;

	float nx = PosTex.x * row1.x + PosTex.y * row2.x + spriteInfo->destination.x;
	float ny = PosTex.x * row1.y + PosTex.y * row2.y + spriteInfo->destination.y;
	
	PosTex.x = nx;
	PosTex.y = ny;

	PosTex.z *= spriteInfo->source.z;
	PosTex.w *= spriteInfo->source.w;
		
	PosTex.z += spriteInfo->source.x;
	PosTex.w += spriteInfo->source.y;

	data[index * SpriteBatch::VerticesPerSprite + i].position = glm::vec3(PosTex.x, PosTex.y, 0.0f);
	data[index * SpriteBatch::VerticesPerSprite + i].texcoord = glm::vec2(PosTex.z, PosTex.w);
	data[index * SpriteBatch::VerticesPerSprite + i].color = glm::vec4(spriteInfo->color.x, spriteInfo->color.y,
						       spriteInfo->color.z, spriteInfo->color.w);
    }
}

void SpriteBatch::sortSprites()
{
    for(uint32_t i = 0; i < spriteQueueCount; i++)
    {
	spriteSortList[i] = &spriteQueue[i];
    }

    qsort(spriteSortList, spriteQueueCount, sizeof(SpriteInfo*), [](const void* a, const void* b)
	  -> int
	  {
	      SpriteInfo* sa = *(SpriteInfo**)a;
	      SpriteInfo* sb = *(SpriteInfo**)b;

	      // Sort front to back
	      if (sa->originRotationDepth.w > sb->originRotationDepth.w)
		  return -1;
	      else if (sa->originRotationDepth.w < sb->originRotationDepth.w)
		  return 1;

	      // Sort by texture
	      if (sa->texture.index > sb->texture.index)
		  return -1;
	      else if (sa->texture.index < sb->texture.index)
		  return 1;
	      else
		  return 0;
	      
	  });
}
