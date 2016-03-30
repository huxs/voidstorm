
const float Renderer::GaussBlurSigma = 3.0f;
const float Renderer::GaussBlurTapSize = 1.0f;
const float Renderer::Exposure = 0.0f;
const float Renderer::ZoomFactor = 0.75f;

struct Vertex1P1UV
{
    glm::vec3 pos;
    glm::vec2 uv;
};

Renderer::Renderer(dcfx::Context *renderContext, GameMemory *memory)
	: renderCtx(renderContext)
{
    isFullscreen = false;
    resolution.x = 1280;
    resolution.y = 720;
    aspectRatio = (float)resolution.y / resolution.x;
	    
    spritebatch = new(memory->permStackAllocator->alloc(sizeof(SpriteBatch)))  SpriteBatch(renderCtx);
    linerenderer = new(memory->permStackAllocator->alloc(sizeof(LineRenderer)))  LineRenderer(renderCtx);
    particle = new(memory->permStackAllocator->alloc(sizeof(ParticleEngine))) ParticleEngine(
	memory->gameStackAllocator,
	renderCtx,
	spritebatch);
    
    Vertex1P1UV* verts = (Vertex1P1UV*)renderCtx->frameAlloc(4 * sizeof(Vertex1P1UV));
    verts[0].pos = glm::vec3(-1.0f, -1.0f, 0.0f);
    verts[1].pos = glm::vec3(+1.0f, -1.0f, 0.0f);
    verts[2].pos = glm::vec3(-1.0f, +1.0f, 0.0f);
    verts[3].pos = glm::vec3(+1.0f, +1.0f, 0.0f);

    verts[0].uv = glm::vec2(0.0f, 0.0f);
    verts[1].uv = glm::vec2(1.0f, 0.0f);
    verts[2].uv = glm::vec2(0.0f, 1.0f);
    verts[3].uv = glm::vec2(1.0f, 1.0f);
		
    uint32_t* indices = (uint32_t*)renderCtx->frameAlloc(6 * sizeof(uint32_t));
    indices[0] = 0; 
    indices[1] = 1; 
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 1; 
    indices[5] = 3; 

    dcfx::VertexDecl decl;
    decl.begin();
    decl.add(0, 3, dcfx::AttributeType::FLOAT, false);
    decl.add(1, 2, dcfx::AttributeType::FLOAT, false);
    decl.end();

    fsqVertexBuffer = renderCtx->createVertexBuffer(&verts[0], 4 * sizeof(Vertex1P1UV), decl);
    fsqIndexBuffer = renderCtx->createBuffer(&indices[0], 6 * sizeof(uint32_t), dcfx::BufferType::INDEX);

    createFramebuffers();
   
    colorSampler = renderCtx->createUniform("Scene", dcfx::UniformType::SAMPLER, 1);
    sigmaUniform = renderCtx->createUniform("Sigma", dcfx::UniformType::FLOAT, 1);
    tapSizeUniform = renderCtx->createUniform("TapSize", dcfx::UniformType::FLOAT, 1);
    exposureUniform = renderCtx->createUniform("Exposure", dcfx::UniformType::FLOAT, 1);

    // TODO (daniel): Use a resource manager for shaders so we don't have to load the same once over again..
    dcfx::ProgramDesc desc;
    desc.m_vert = loadShader(renderCtx, "fullscreenquad.vert", dcfx::ShaderType::VERTEX);
    desc.m_frag = loadShader(renderCtx, "blur_vertical.frag", dcfx::ShaderType::FRAGMENT);
    blurVerticalProgram = renderCtx->createProgram(desc);

    desc.m_vert = loadShader(renderCtx, "fullscreenquad.vert", dcfx::ShaderType::VERTEX);
    desc.m_frag = loadShader(renderCtx, "blur_horizontal.frag", dcfx::ShaderType::FRAGMENT);
    blurHorizontalProgram = renderCtx->createProgram(desc);

    desc.m_vert = loadShader(renderCtx, "fullscreenquad.vert", dcfx::ShaderType::VERTEX);
    desc.m_frag = loadShader(renderCtx, "exposure.frag", dcfx::ShaderType::FRAGMENT);
    exposureProgram = renderCtx->createProgram(desc);

    desc.m_vert = loadShader(renderCtx, "fullscreenquad.vert", dcfx::ShaderType::VERTEX);
    desc.m_frag = loadShader(renderCtx, "fullscreenquad.frag", dcfx::ShaderType::FRAGMENT);
    outputProgram = renderCtx->createProgram(desc);

    defaultValues();
}

Renderer::~Renderer()
{
    renderCtx->deleteBuffer(fsqVertexBuffer);
    renderCtx->deleteBuffer(fsqIndexBuffer);

    deleteFramebuffers();
    
    renderCtx->deleteUniform(colorSampler);
    renderCtx->deleteUniform(sigmaUniform);
    renderCtx->deleteUniform(tapSizeUniform);
    renderCtx->deleteUniform(exposureUniform);
	
    renderCtx->deleteProgram(blurVerticalProgram);
    renderCtx->deleteProgram(blurHorizontalProgram);
    renderCtx->deleteProgram(exposureProgram);
    renderCtx->deleteProgram(outputProgram);

    particle->~ParticleEngine();
    linerenderer->~LineRenderer();
    spritebatch->~SpriteBatch();
}

void Renderer::setResolution(glm::ivec2 _resolution)
{
    resolution = _resolution;
    aspectRatio = (float)resolution.y / resolution.x;
    
    deleteFramebuffers();
    createFramebuffers();   
}

glm::ivec2 Renderer::getResolution()
{
    return resolution;
}

void Renderer::toogleFullscreen()
{
    isFullscreen = (isFullscreen ? false : true);
    // change res
}

void Renderer::setCameraPosition(const glm::vec2& position)
{
    renderCamera.position = position;
}

void Renderer::setPostProcessParams(float blurSigma, float blurTapSize, float exposure)
{
    renderCtx->setUniform(sigmaUniform, &blurSigma, sizeof(GaussBlurSigma));
    renderCtx->setUniform(tapSizeUniform, &blurTapSize, sizeof(GaussBlurTapSize));
    renderCtx->setUniform(exposureUniform, &exposure, sizeof(Exposure));
}

void Renderer::write(const char* text, const glm::vec2& position, bool32 inWorld)
{
    if(inWorld)
    {
	bufferedTextInWorld.push_back({ text, position });
    }
    else
    {
        glm::vec2 positionViewport = position * (glm::vec2)resolution;
	
	bufferedText.push_back({ text, positionViewport });
    }
}

void Renderer::render(World *world)
{
    TIME_BLOCK(Render);

    static const uint32_t LinearSampler =
	(DCFX_SAMPLER_MAG_LINEAR | DCFX_SAMPLER_MIN_LINEAR | DCFX_EDGE_FUNC(
	    DCFX_SAMPLER_EDGE_CLAMP, DCFX_SAMPLER_EDGE_CLAMP, DCFX_SAMPLER_EDGE_REPEAT));

    static const uint32_t PointSampler =
	(DCFX_SAMPLER_MAG_NEAREST | DCFX_SAMPLER_MIN_NEAREST | DCFX_EDGE_FUNC(
	    DCFX_SAMPLER_EDGE_CLAMP, DCFX_SAMPLER_EDGE_CLAMP, DCFX_SAMPLER_EDGE_REPEAT));
    
    const glm::mat4& viewportTransform = getViewportTransform();
    const glm::mat4& viewportCameraTransform = viewportTransform * getCameraTransform();

    renderCtx->setView(RENDERPASS_OUTPUT, viewportTransform);
    renderCtx->setView(RENDERPASS_WORLD, viewportCameraTransform);
    renderCtx->setView(RENDERPASS_DEBUG, viewportCameraTransform);

    spritebatch->setSamplerState(LinearSampler);
    spritebatch->setBlendState(DCFX_DEFAULT_STATE_ADDATIVE);

    for(uint32_t i = 0; i < bufferedText.size(); ++i)
    {
	BufferedText& text = bufferedText[i];	
	spritebatch->write(text.text.c_str(), text.position);
    }

    spritebatch->flush(RENDERPASS_OUTPUT);
    
    // Render all sprites, in world text and particles to a offscreen render target
    renderCtx->setFramebuffer(RENDERPASS_WORLD, sceneFramebuffer);
    renderCtx->setViewport(RENDERPASS_WORLD, glm::ivec4(0, 0, resolution.x, resolution.y));

    for(uint32_t i = 0; i < bufferedTextInWorld.size(); ++i)
    {
	BufferedText& text = bufferedTextInWorld[i];
	spritebatch->write(text.text.c_str(), text.position);
    }
    
    for(uint32_t i = 1; i < world->sprites->data.used; ++i)
    {
	Entity e = world->sprites->data.entities[i];
	glm::vec4 color = world->sprites->data.color[i];
	Texture* texture = world->sprites->data.texture[i];
	glm::vec2 size = world->sprites->data.size[i];
	glm::vec2 origin = world->sprites->data.origin[i];

	if(texture == NULL)
	    continue;
	
	TransformManager::Instance transform = world->transforms->lookup(e);
	assert(transform.index != 0);

	glm::vec2 pos = world->transforms->data.position[transform.index];
	float rotation = world->transforms->data.rotation[transform.index];
	float depth = world->transforms->data.depth[transform.index];
	float cscale = world->transforms->data.scale[transform.index]; 

	// Perspective projection
	glm::vec2 projectedXY = (1.0f / depth) * pos;
	float scale = 1.0f / depth;
	
	spritebatch->setDestination(glm::vec4(projectedXY.x, projectedXY.y,
					      size.x * cscale * scale, size.y * cscale * scale));
	spritebatch->setRotation(rotation);
	spritebatch->setOrigin(origin);
	spritebatch->setTexture(texture->handle);
	spritebatch->setSource(glm::vec4(0,0,1,1));
	spritebatch->setColor(color);
	spritebatch->setDepth(depth);
	spritebatch->submit();
    }

    particle->render(RENDERPASS_WORLD);
    
    spritebatch->flush(RENDERPASS_WORLD);

    spritebatch->setSamplerState(PointSampler);
    
    // Render using fullscreen quad
    renderCtx->setState(DCFX_DEFAULT_STATE_NOBLEND);
    renderCtx->setVertexBuffer(fsqVertexBuffer);
    renderCtx->setIndexBuffer(fsqIndexBuffer, 0, 6);
    
    // Downsample
    renderCtx->setFramebuffer(RENDERPASS_POSTPROCESS, blurFramebuffer0);
    renderCtx->setViewport(RENDERPASS_POSTPROCESS, glm::ivec4(0, 0, blurSizeX, blurSizeY));
    renderCtx->setTexture(colorSampler, sceneTexture, SAMPLER_STATE);
    renderCtx->setProgram(outputProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS);		

    // Blur horizontal
    renderCtx->setFramebuffer(RENDERPASS_POSTPROCESS + 1, blurFramebuffer1);
    renderCtx->setViewport(RENDERPASS_POSTPROCESS + 1, glm::ivec4(0, 0, blurSizeX, blurSizeY));
    renderCtx->setTexture(colorSampler, blurTexture0, SAMPLER_STATE);
    renderCtx->setProgram(blurHorizontalProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS + 1);

    // Blur vertical
    renderCtx->setFramebuffer(RENDERPASS_POSTPROCESS + 2, blurFramebuffer0);
    renderCtx->setViewport(RENDERPASS_POSTPROCESS + 2, glm::ivec4(0, 0, blurSizeX, blurSizeY));
    renderCtx->setTexture(colorSampler, blurTexture1, SAMPLER_STATE);
    renderCtx->setProgram(blurVerticalProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS + 2);	

#if 1
    // Render to HDR target and do color correction.
    // Render blurred result.
    renderCtx->setFramebuffer(RENDERPASS_POSTPROCESS + 3, exposureFramebuffer);
    renderCtx->setViewport(RENDERPASS_POSTPROCESS + 3, glm::ivec4(0, 0, resolution.x, resolution.y));
    renderCtx->setTexture(colorSampler, blurTexture0, SAMPLER_STATE);
    renderCtx->setProgram(outputProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS + 3);	

    renderCtx->setState(
	(DCFX_STATE_WRITE_ALL |
	 DCFX_STATE_ADDATIVE_BLEND |
	 DCFX_STATE_CULL_CCW |
	 DCFX_STATE_DEPTH_TEST_ALWAYS));
    
    // Render original scene
    renderCtx->setViewport(RENDERPASS_POSTPROCESS + 3, glm::ivec4(0, 0, resolution.x, resolution.y));
    renderCtx->setTexture(colorSampler, sceneTexture, SAMPLER_STATE);
    renderCtx->setProgram(outputProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS + 3);

    // Output to backbuffer
    renderCtx->setViewport(RENDERPASS_OUTPUT, glm::ivec4(0, 0, resolution.x, resolution.y));
    renderCtx->setTexture(colorSampler, exposureTexture, SAMPLER_STATE);
    renderCtx->setProgram(exposureProgram);
    renderCtx->submit(RENDERPASS_OUTPUT);
    
#else
    renderCtx->setViewport(RENDERPASS_OUTPUT, glm::ivec4(0, 0, resolution.x, resolution.y));
    renderCtx->setTexture(colorSampler, sceneTexture, SAMPLER_STATE);
    renderCtx->setProgram(exposureProgram);
    renderCtx->submit(RENDERPASS_OUTPUT);
#endif

    //Render debug information such as lines, graphs etc
    linerenderer->dispatch(RENDERPASS_DEBUG);
}

void Renderer::frame()
{
    TIME_BLOCK(Swap);
    
    spritebatch->reset();
    linerenderer->reset();
    bufferedText.clear();
    bufferedTextInWorld.clear();
    
    renderCtx->frame();
}

void Renderer::deleteFramebuffers()
{
    renderCtx->deleteFramebuffer(sceneFramebuffer);
    renderCtx->deleteFramebuffer(blurFramebuffer0);
    renderCtx->deleteFramebuffer(blurFramebuffer1);
    renderCtx->deleteFramebuffer(exposureFramebuffer);
    
    renderCtx->deleteTexture(sceneTexture);
    renderCtx->deleteTexture(blurTexture0);
    renderCtx->deleteTexture(blurTexture1);
    renderCtx->deleteTexture(exposureTexture);
}

void Renderer::createFramebuffers()
{    
    blurSizeX = resolution.x / 2;
    blurSizeY = resolution.y / 2;
	
    sceneTexture = renderCtx->createTexture(resolution.x, resolution.y, dcfx::TextureFormat::RGBA32F);
    blurTexture0 = renderCtx->createTexture(blurSizeX, blurSizeY, dcfx::TextureFormat::RGBA32F);
    blurTexture1 = renderCtx->createTexture(blurSizeX, blurSizeY, dcfx::TextureFormat::RGBA32F);
    exposureTexture = renderCtx->createTexture(resolution.x, resolution.y, dcfx::TextureFormat::RGBA32F);

    sceneFramebuffer = renderCtx->createFramebuffer(&sceneTexture, 1);	
    blurFramebuffer0 = renderCtx->createFramebuffer(&blurTexture0, 1);
    blurFramebuffer1 = renderCtx->createFramebuffer(&blurTexture1, 1);	
    exposureFramebuffer = renderCtx->createFramebuffer(&exposureTexture, 1);
}

void Renderer::defaultValues()
{
    renderCtx->setUniform(sigmaUniform, &GaussBlurSigma, sizeof(GaussBlurSigma));
    renderCtx->setUniform(tapSizeUniform, &GaussBlurTapSize, sizeof(GaussBlurTapSize));
    renderCtx->setUniform(exposureUniform, &Exposure, sizeof(Exposure));   
}
