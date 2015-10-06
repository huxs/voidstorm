#include <SDL2/SDL_syswm.h>

const float Renderer::GaussBlurSigma = 1.6f;
const float Renderer::GaussBlurTapSize = 1.0f;
const float Renderer::Exposure = 5.0f;

// TODO: Not a big fan of this constant.
const float Renderer::CameraZoom = 1280.0f;

struct Vertex1P1UV
{
    glm::vec3 pos;
    glm::vec2 uv;
};

Renderer::Renderer(HeapAllocator* heap, dcutil::Stack* perm, dcutil::Stack* world)
{
    resolution.x = 1280;
    resolution.y = 720;
    isFullscreen = false;
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    int glContextFlags = 0;
#if defined (_DEBUG)
    glContextFlags = SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, glContextFlags);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    Uint32 createWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("Voidstorm",
			      SDL_WINDOWPOS_UNDEFINED,
			      SDL_WINDOWPOS_UNDEFINED,
			      resolution.x,
			      resolution.y,
			      createWindowFlags);

   
    if (window == nullptr) {
	PRINT("Failed to create window %s. \n", SDL_GetError());
	return;
    }
    
    SDL_GetWindowSize(window, &resolution.x, &resolution.y);

    metersToPixels = ((float)resolution.x / CameraZoom);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    HWND hwnd = info.info.win.window;
    
    renderCtx = new(perm->alloc(sizeof(dcfx::Context))) dcfx::Context(hwnd, heap);
    spritebatch = new(perm->alloc(sizeof(SpriteBatch)))  SpriteBatch(renderCtx);
    linerenderer = new(perm->alloc(sizeof(LineRenderer)))  LineRenderer(renderCtx);
    particle = new(perm->alloc(sizeof(ParticleEngine))) ParticleEngine(world, renderCtx, spritebatch);
    
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
    renderCtx->~Context();

    SDL_DestroyWindow(window);
}

void Renderer::setResolution(glm::ivec2 resolution)
{
#ifndef ANDROID    
    this->resolution = resolution;

    // TODO: This is a slow way of resizing the framebuffers.
    deleteFramebuffers();
    createFramebuffers();

    metersToPixels = ((float)resolution.x / CameraZoom);
#endif    
}

glm::ivec2 Renderer::getResolution()
{
    return resolution;
}

void Renderer::toogleFullscreen()
{
    isFullscreen = (isFullscreen ? false : true);
    SDL_SetWindowFullscreen(window, isFullscreen);
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
    spritebatch->write(text, position);
}

void Renderer::render(World* world)
{
    TIME_BLOCK(Render);
    
    const glm::mat4& viewportTransform = getViewportTransform();
    const glm::mat4& viewportCameraTransform = viewportTransform * getCameraTransform();

    renderCtx->setView(RENDERPASS_OUTPUT, viewportTransform);
    renderCtx->setView(RENDERPASS_WORLD, viewportCameraTransform);
    renderCtx->setView(RENDERPASS_DEBUG, viewportCameraTransform);

    // NOTE: Draws fonts to output target.
    spritebatch->draw(RENDERPASS_OUTPUT);

    // NOTE: Render all sprites to a offscreen render target for post-processing.
    renderCtx->setFramebuffer(RENDERPASS_WORLD, sceneFramebuffer);
    renderCtx->setViewport(RENDERPASS_WORLD, glm::ivec4(0, 0, resolution.x, resolution.y));

    spritebatch->setBlendState(DCFX_DEFAULT_STATE_ADDATIVE);
    spritebatch->setSortMode(SpriteSortMode::BACKTOFRONT);	

    for(uint32_t i = 1; i < world->sprites.data.used; ++i)
    {
	Entity e = world->sprites.data.entities[i];
	glm::vec4 color = world->sprites.data.color[i];
	Texture* texture = world->sprites.data.texture[i];
	glm::vec2 size = world->sprites.data.size[i];
	glm::vec2 origin = world->sprites.data.origin[i];
	
	TransformManager::Instance transform = world->transforms.lookup(e);
	assert(transform.index != 0);

	glm::vec2 pos = world->transforms.data.position[transform.index];
	float rotation = world->transforms.data.rotation[transform.index];
	float depth = world->transforms.data.depth[transform.index];
	float cscale = world->transforms.data.scale[transform.index];

	// Perspective projection.
	glm::vec2 projectedXY = (1.0f / depth) * pos;
	float scale = (1.0f/depth)*cscale;
	
	spritebatch->setDestination(glm::vec4(projectedXY.x, projectedXY.y,
					      size.x*scale, size.y*scale));
	spritebatch->setRotation(rotation);
	spritebatch->setOrigin(origin);
	if(texture != NULL)
	spritebatch->setTexture(texture->handle);
	spritebatch->setSource(glm::vec4(0,0,1,1));
	spritebatch->setColor(color);
	spritebatch->setDepth((float)i);
	spritebatch->submit();
    }

    // TODO: Sort sprtes and particles together.
    spritebatch->draw(RENDERPASS_WORLD);

    particle->render(RENDERPASS_WORLD);

    // NOTE: Render using fullscreen quad.
    renderCtx->setState(DCFX_DEFAULT_STATE_NOBLEND);
    renderCtx->setVertexBuffer(fsqVertexBuffer);
    renderCtx->setIndexBuffer(fsqIndexBuffer, 0, 6);
    
    // NOTE: Downsample 4x.
    renderCtx->setFramebuffer(RENDERPASS_POSTPROCESS, blurFramebuffer0);
    renderCtx->setViewport(RENDERPASS_POSTPROCESS, glm::ivec4(0, 0, blurSizeX, blurSizeY));
    renderCtx->setTexture(colorSampler, sceneTexture, SAMPLER_STATE);
    renderCtx->setProgram(outputProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS);		

    // NOTE: Blur horizontal.
    renderCtx->setFramebuffer(RENDERPASS_POSTPROCESS + 1, blurFramebuffer1);
    renderCtx->setViewport(RENDERPASS_POSTPROCESS + 1, glm::ivec4(0, 0, blurSizeX, blurSizeY));
    renderCtx->setTexture(colorSampler, blurTexture0, SAMPLER_STATE);
    renderCtx->setProgram(blurHorizontalProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS + 1);

    // NOTE: Blur vertical.
    renderCtx->setFramebuffer(RENDERPASS_POSTPROCESS + 2, blurFramebuffer0);
    renderCtx->setViewport(RENDERPASS_POSTPROCESS + 2, glm::ivec4(0, 0, blurSizeX, blurSizeY));
    renderCtx->setTexture(colorSampler, blurTexture1, SAMPLER_STATE);
    renderCtx->setProgram(blurVerticalProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS + 2);	

#ifndef ANDROID    
    // NOTE: Render to HDR target and do color correction.
    // NOTE: Render blurred result.
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
    
    // NOTE: Render original scene.
    renderCtx->setViewport(RENDERPASS_POSTPROCESS + 3, glm::ivec4(0, 0, resolution.x, resolution.y));
    renderCtx->setTexture(colorSampler, sceneTexture, SAMPLER_STATE);
    renderCtx->setProgram(outputProgram);
    renderCtx->submit(RENDERPASS_POSTPROCESS + 3);

    // NOTE: Output to backbuffer.
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
    
    // NOTE: Render debug information such as lines, graphs etc.
    linerenderer->dispatch(RENDERPASS_DEBUG);
}

void Renderer::frame()
{
    TIME_BLOCK(Swap);
    
    spritebatch->reset();
    linerenderer->reset();
    
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
    blurSizeX = resolution.x / 4;
    blurSizeY = resolution.y / 4;
	
    sceneTexture = renderCtx->createTexture(resolution.x, resolution.y, dcfx::TextureFormat::RGBA8);
    blurTexture0 = renderCtx->createTexture(blurSizeX, blurSizeY, dcfx::TextureFormat::RGBA8);
    blurTexture1 = renderCtx->createTexture(blurSizeX, blurSizeY, dcfx::TextureFormat::RGBA8);
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
