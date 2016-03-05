#pragma once

#include "voidstorm_spritebatch.h"
#include "voidstorm_linerender.h"
#include "voidstorm_particle.h"

#define RENDERPASS_WORLD 0
#define RENDERPASS_POSTPROCESS 1
#define RENDERPASS_OUTPUT 6
#define RENDERPASS_DEBUG 7

struct Camera
{
    glm::vec2 position;
};

struct World;
class Renderer
{
public:
    Renderer(HeapAllocator* heap);
    ~Renderer();

    void setResolution(glm::ivec2 resolution);
    glm::ivec2 getResolution();
    void toogleFullscreen();

    void setCameraPosition(const glm::vec2& position);
    
    void setPostProcessParams(float blurSigma, float blurTapSize, float exposure);
    
    dcfx::Context* getContext() { return renderCtx; }
    LineRenderer* getLineRenderer() { return linerenderer; }
    ParticleEngine* getParticleEngine() { return particle; } 

    void write(const char* text, const glm::vec2& position, bool32 inWorld);
    
    void render(World* world);
    void frame();
    
private:
    void deleteFramebuffers();
    void createFramebuffers();
    void defaultValues();

    glm::mat4 getCameraTransform();
    glm::mat4 getViewportTransform();
    
    static const float GaussBlurSigma;
    static const float GaussBlurTapSize;
    static const float Exposure;
    static const float ZoomFactor;

    SDL_Window* window;   
    dcfx::Context* renderCtx;
    SpriteBatch* spritebatch;
    LineRenderer* linerenderer;
    ParticleEngine* particle;

    Camera renderCamera;
    glm::ivec2 resolution;
    float aspectRatio;
    bool isFullscreen;

    struct BufferedText
    {
	tinystl::string text;
	glm::vec2 position;
    };
    
    tinystl::vector<BufferedText> bufferedText;
    tinystl::vector<BufferedText> bufferedTextInWorld;
    
    dcfx::BufferHandle fsqVertexBuffer;
    dcfx::BufferHandle fsqIndexBuffer;
    int blurSizeX;
    int blurSizeY;
	
    dcfx::UniformHandle colorSampler;
    dcfx::UniformHandle sigmaUniform;
    dcfx::UniformHandle tapSizeUniform;
    dcfx::UniformHandle exposureUniform;

    dcfx::TextureHandle blurTexture0;
    dcfx::TextureHandle blurTexture1;
    dcfx::TextureHandle exposureTexture;
    dcfx::TextureHandle sceneTexture;
	
    dcfx::FramebufferHandle sceneFramebuffer;
    dcfx::FramebufferHandle blurFramebuffer0;
    dcfx::FramebufferHandle blurFramebuffer1;;
    dcfx::FramebufferHandle exposureFramebuffer;
	
    dcfx::ProgramHandle blurVerticalProgram;
    dcfx::ProgramHandle blurHorizontalProgram;
    dcfx::ProgramHandle exposureProgram;
    dcfx::ProgramHandle outputProgram;
};

inline glm::mat4 Renderer::getCameraTransform()
{
    float D = 3000.0f * ZoomFactor;

    float zf = (2.0f / D) / ((2.0f / resolution.x));
    
    /* Offsets the origin so that camera position 0,0 looks at center of the viewport. */
    return glm::mat4(
        zf, 0.0f, 0.0f, 0.0f,
	0.0f, zf, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
        -renderCamera.position.x*zf + (resolution.x / 2.0f),
	-renderCamera.position.y*zf + (resolution.y / 2.0f), 0.0f, 1.0f
	);
}

inline glm::mat4 Renderer::getViewportTransform()
{
    /* Scale the ranges 0 - Width, 0 - Height into the unit cube -1 - 1.
       Reverse the y-range. Then offset the range by -1 in x and by 1 in y.
       This makes the following ranges transform into viewport.
	   
       0,0          0,Width
	   

       0,Height     Width,Height */
    
    return glm::mat4(
        2.0f / resolution.x, 0.0f, 0.0f, 0.0f,
	0.0f, -2.0f / resolution.y, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 1.0f
	);
}
