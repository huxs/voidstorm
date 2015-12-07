#pragma once

#define OPENGL
//#define D3D12

#include "dcfx_helper.h"
#include "config.h"
#include "dcfx/defines.h"
#include "vertexdecl.h"
#include "image_parser.h"

#include <dcutil/handleallocator.h>
#include <dcutil/bytebuffer.h>
#include <dcutil/allocator.h>
#include <dcutil/semaphore.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <thread>
#include <atomic>
#include <stdint.h>
typedef int bool32;

#define TINYSTL_ALLOCATOR dcfx::TinyStlAllocator
#include <TINYSTL/unordered_map.h>

namespace dcfx
{
    struct TinyStlAllocator
    {
	static void* static_allocate(size_t _bytes);
	static void static_deallocate(void* _ptr, size_t);
    };

    struct CallbackI
    {
	virtual void profile(double ms) = 0;
    };

    struct PlatformData
    {
	void* m_window;
    };
    
    extern CallbackI* g_callback;
    extern dcutil::AllocatorI* g_allocator;
    extern PlatformData g_platformData;
    
    enum class Cmd : uint8_t
    {
	RendererInit,
	RendererShutdown,
	CreateVertexBuffer,
	CreateBuffer,
	DestroyBuffer,
	UpdateBuffer,
	ReadBuffer,
	CreateVertexDecl,
	DestroyVertexDecl,
	CreateShader,
	DestroyShader,
	CreateProgram,
	DestroyProgram,
	CreateUniform,
	DestroyUniform,
	CreateEmptyTexture,
	CreateTexture,
	DestroyTexture,
	UpdateTexture,
	ReadTexture,
	GenerateMips,
	CreateSampler,
	DestroySampler,
	CreateFramebuffer,
	DestroyFramebuffer,
	End
    };

    typedef dcutil::ByteBuffer<DCFX_MAX_COMMAND_BUFFER_SIZE> CommandBuffer;
    typedef dcutil::ByteBuffer<DCFX_MAX_UNIFORM_BUFFER_SIZE> UniformBuffer;

    static const uint16_t InvalidHandle = UINT16_MAX;

#define VALID_HANDLE(handle) ((handle.index == InvalidHandle) ? false : true)

    struct VertexDeclHandle { uint16_t index; };
    struct BufferHandle { uint16_t index; };
    struct ShaderHandle { uint16_t index; };
    struct ProgramHandle { uint16_t index; };
    struct UniformHandle { uint16_t index; };
    struct TextureHandle { uint16_t index; };
    struct SamplerHandle { uint16_t index; };
    struct FramebufferHandle { uint16_t index; };

    template<class T>
    static const T Invalid() { return { InvalidHandle }; }

    class MatrixCache
    {
    public:
	MatrixCache()
		: m_count(1)
	    {
		m_matrices[0] = glm::mat4(1.0f);
	    }
	
	uint32_t add(const glm::mat4& mat);
	const glm::mat4& get(uint32_t index) const;
	void reset();
    private:
	glm::mat4 m_matrices[DCFX_MATRIX_CACHE_COUNT];
	uint32_t m_count;
    };

    // Struct describing how to bind a layer of a texture
    struct ImageBind
    {
	enum class Access
	{
	    READ,
	    WRITE,
	    READWRITE
	};
	
	ImageBind()
		:
		m_handle({ InvalidHandle }),
		m_access(Access::READWRITE),
		m_layered(false),
		m_layer(0),
		m_level(0)
	    {}
	
	TextureHandle m_handle;
	Access m_access;
	bool m_layered;
	uint32_t m_layer;
	uint32_t m_level;	
    };

/*
  Sortkey layout:
  View (8bit) Item Type (4bit) ProgramID (16bit) Unused (36bit)
*/
#define SORTKEY_VIEW_MASK UINT64_C(0xFF00000000000000)
#define SORTKEY_VIEW_SHIFT 56
#define SORTKEY_ITEM_MASK UINT64_C(0x00F0000000000000)		
#define SORTKEY_ITEM_SHIFT 52
#define SORTKEY_PROGRAM_MASK UINT64_C(0x000FFFF000000000)
#define SORTKEY_PROGRAM_SHIFT 36
    
    struct ComputeCall
    {
	ComputeCall()
		:
		m_uniformBegin(0),
		m_uniformEnd(0),
		m_numX(0),
		m_numY(0),
		m_numZ(0),
		m_imageCount(0),
		m_bufferCount(0),
		m_textureCount(0)
	    {
		for(uint16_t i = 0; i < DCFX_MAX_COMPUTE_BUFFER_BINDINGS; i++)
		{
		    m_buffers[i] = { InvalidHandle };
		}

		for(uint16_t i = 0; i < DCFX_MAX_TEXTURE_SAMPLERS; i++)
		{
		    m_textures[i] = { InvalidHandle };
		    m_samplers[i] = { InvalidHandle };
		    m_samplerStates[i] = 0;
		}
	    }

	size_t m_uniformBegin;
	size_t m_uniformEnd;

	uint16_t m_numX;
	uint16_t m_numY;
	uint16_t m_numZ;

	ImageBind m_images[DCFX_MAX_IMAGE_UNITS];
	uint16_t m_imageCount;

	BufferHandle m_buffers[DCFX_MAX_COMPUTE_BUFFER_BINDINGS];
	uint16_t m_bufferCount;

	TextureHandle m_textures[DCFX_MAX_TEXTURE_SAMPLERS];
	SamplerHandle m_samplers[DCFX_MAX_TEXTURE_SAMPLERS];
	uint32_t m_samplerStates[DCFX_MAX_TEXTURE_SAMPLERS];	
	uint16_t m_textureCount;
    };
    
    struct DrawCall
    {
	DrawCall()
		: 
		m_vertexBufferHandle({ InvalidHandle }),
		m_indexBufferHandle({ InvalidHandle }),
		m_instanceBufferHandle({ InvalidHandle }),
		m_baseVertex(0),
		m_indexOffset(0),
		m_count(UINT32_MAX),
		m_uniformBegin(0),
		m_uniformEnd(0),
		m_transform(0),
		m_state(0),
		m_primitiveMode(DCFX_STATE_PT_TRIANGLES),
		m_numInstances(1),
		m_instanceOffset(0),
		m_instanceStride(0),
		m_textureCount(0)
	    {
		for(uint16_t i = 0; i < DCFX_MAX_TEXTURE_SAMPLERS; i++)
		{
		    m_textures[i] = { InvalidHandle };
		    m_samplers[i] = { InvalidHandle };
		    m_samplerStates[i] = 0;
		}
	    }

	uint32_t m_baseVertex;
	uint32_t m_indexOffset;
	uint32_t m_count;
	size_t m_uniformBegin;
	size_t m_uniformEnd;
	uint32_t m_transform;
	uint32_t m_state;
	uint32_t m_primitiveMode;
	uint32_t m_numInstances;
	uint32_t m_instanceOffset;
	uint16_t m_instanceStride;
	
	BufferHandle m_vertexBufferHandle;
	BufferHandle m_indexBufferHandle;
	BufferHandle m_instanceBufferHandle;

	TextureHandle m_textures[DCFX_MAX_TEXTURE_SAMPLERS];
	SamplerHandle m_samplers[DCFX_MAX_TEXTURE_SAMPLERS];
	uint32_t m_samplerStates[DCFX_MAX_TEXTURE_SAMPLERS];
	uint16_t m_textureCount;
    };

    struct RenderItem
    {
	ComputeCall m_compute;
	DrawCall m_draw;
    };

    struct Frame
    {
	Frame()
		:
		m_freeUniformHandlesCount(0),
		m_freeVertexDeclHandlesCount(0),
		m_freeBufferHandlesCount(0),
		m_freeShaderHandlesCount(0),
		m_freeProgramHandlesCount(0),
		m_freeTextureHandlesCount(0),
		m_freeSamplerHandlesCount(0),
		m_freeFramebufferHandlesCount(0)
	    {
		for(uint16_t i = 0; i < DCFX_MAX_VIEWS; i++)
		{
		    for(uint16_t j = 0; j < DCFX_MAX_TEXTURES_TO_MIP_PER_VIEW; ++j)
		    {
			m_texturesToMip[i][j].index = InvalidHandle;
		    }

		    m_texturesToMipCount[i] = 0;
		    m_blitTargets[i].index = InvalidHandle;
		}
	    }

	void initializeAllocator(void* mem, uint32_t size);
	
	void start();
	void finnish();

	uint16_t submit(uint16_t view, uint32_t flag);
	uint16_t dispatch(uint16_t view, ProgramHandle handle, uint16_t numX, uint16_t numY, uint16_t numZ);
	void sort();

	void setVertexBuffer(BufferHandle handle, uint32_t baseVertex, uint32_t count);
	void setIndexBuffer(BufferHandle handle, uint32_t indexOffset, uint32_t count);
	void setInstanceBuffer(BufferHandle handle, uint32_t numInstances, uint16_t stride, uint32_t offset = 0);
	void setProgram(ProgramHandle handle);
	void setTransform(const glm::mat4& transform);
	void setState(uint32_t state);
	void setPrimitiveMode(uint32_t mode);
	void setFramebuffer(uint16_t index, FramebufferHandle handle);
	void setViewport(uint16_t index, const glm::ivec4& viewport); 
	void setClearColor(uint16_t index, const glm::vec4& clearColor);
	void setTexture(UniformHandle uniform, TextureHandle texture, uint32_t state);
	void setTexture(UniformHandle uniform, TextureHandle texture, SamplerHandle sampler);
	void setImage(UniformHandle sampler, TextureHandle texture, ImageBind::Access access, uint8_t mip, bool layered, uint8_t layer);
	void setBuffer(uint16_t binding, BufferHandle buffer);
		
	void free(UniformHandle handle);
	void free(VertexDeclHandle handle);
	void free(BufferHandle handle);		
	void free(ShaderHandle handle);
	void free(ProgramHandle handle);
	void free(TextureHandle handle);
	void free(SamplerHandle handle);
	void free(FramebufferHandle handle);

	void clearFreeHandles();

	dcutil::StackAllocator m_allocator;

	CommandBuffer m_commands;
	UniformBuffer m_uniforms;
	MatrixCache m_transforms;

	FramebufferHandle m_blitTargets[DCFX_MAX_VIEWS];
	bool32 m_clear[DCFX_MAX_VIEWS]; // TODO: View flags..
	
	TextureHandle m_texturesToMip[DCFX_MAX_VIEWS][DCFX_MAX_TEXTURES_TO_MIP_PER_VIEW];
	uint8_t m_texturesToMipCount[DCFX_MAX_VIEWS];
	
	FramebufferHandle m_framebuffers[DCFX_MAX_VIEWS];
	glm::vec4 m_clearColor[DCFX_MAX_VIEWS];
	glm::ivec4 m_viewport[DCFX_MAX_VIEWS];
	glm::mat4 m_view[DCFX_MAX_VIEWS];
	glm::mat4 m_proj[DCFX_MAX_VIEWS];

	uint64_t m_sortKeys[DCFX_MAX_DRAW_CALLS];
	uint16_t m_sortValues[DCFX_MAX_DRAW_CALLS];
	RenderItem m_items[DCFX_MAX_DRAW_CALLS];

	uint16_t m_numItems;
	ComputeCall m_compute;
	DrawCall m_draw;

	uint16_t m_freeUniformHandlesCount;
	uint16_t m_freeVertexDeclHandlesCount;
	uint16_t m_freeBufferHandlesCount;
	uint16_t m_freeShaderHandlesCount;
	uint16_t m_freeProgramHandlesCount;
	uint16_t m_freeTextureHandlesCount;
	uint16_t m_freeSamplerHandlesCount;
	uint16_t m_freeFramebufferHandlesCount;

	UniformHandle m_freeUniformHandles[DCFX_MAX_UNIFORMS];
	VertexDeclHandle m_freeVertexDeclHandles[DCFX_MAX_VERTEX_DECLS];
	BufferHandle m_freeBufferHandles[DCFX_MAX_BUFFERS];
	ShaderHandle m_freeShaderHandles[DCFX_MAX_SHADERS];
	ProgramHandle m_freeProgramHandles[DCFX_MAX_PROGRAMS];
	TextureHandle m_freeTextureHandles[DCFX_MAX_TEXTURES];
	SamplerHandle m_freeSamplerHandles[DCFX_MAX_SAMPLERS];
	FramebufferHandle m_freeFramebufferHandles[DCFX_MAX_FRAMEBUFFERS];
    };

    enum class BufferType
    {
	VERTEX,
	INDEX,
	COMPUTE
    };

    enum class ShaderType
    {
	VERTEX,
	FRAGMENT,
	COMPUTE
    };

    static const size_t s_uniformTypeSizes[] = { 4, 4, 12, 16, 4 };

    enum class UniformType
    {
	FLOAT,
	INT,
	VEC3,
	VEC4,
	SAMPLER
    };

    struct ProgramDesc
    {
	ProgramDesc()
		: m_vert({ InvalidHandle }),
		  m_frag({ InvalidHandle }),
		  m_compute({ InvalidHandle })
	    {}
	
	ShaderHandle m_vert;
	ShaderHandle m_frag;
	ShaderHandle m_compute;
    };

    static const char* s_predfinedUniformNames[] =
    { "g_View", "g_InvView", "g_Proj", "g_InvProj", "g_Transform", "g_ViewProjection" };

    enum class PredefinedUniformType
    {
	VIEW,
	INVVIEW,
	PROJ,
	INVPROJ,
	TRANSFORM,
	VIEWPROJ,
	NONE
    };

    PredefinedUniformType getPredefinedUniformType(const char* name);

    struct UniformDesc
    {
	UniformHandle m_handle;
	UniformType m_type;
	uint8_t m_num;
    };

    struct RenderContextI
    {
	virtual ~RenderContextI() {};
	virtual void flip() = 0;
	virtual void render(Frame* frame) = 0;
	virtual void createVertexDecl(VertexDeclHandle handle, const VertexDecl& decl) = 0;
	virtual void deleteVertexDecl(VertexDeclHandle handle) = 0;
	virtual void createVertexBuffer(BufferHandle handle, void* mem, size_t size, VertexDeclHandle decl) = 0;
	virtual void createBuffer(BufferHandle handle, void* mem, size_t size, BufferType type) = 0;
	virtual void deleteBuffer(BufferHandle handle) = 0;
	virtual void updateBuffer(BufferHandle handle, size_t offset, size_t size, void* mem) = 0;
	virtual void readBuffer(BufferHandle handle, size_t offset, size_t size, void* mem) = 0;
	virtual void createShader(ShaderHandle handle, void* mem, size_t size, ShaderType type) = 0;
	virtual void deleteShader(ShaderHandle handle) = 0;
	virtual void createProgram(ProgramHandle handle, ProgramDesc desc) = 0;
	virtual void deleteProgram(ProgramHandle handle) = 0;
	virtual void createUniform(UniformHandle handle, const char* name, UniformType type, uint8_t num) = 0;
	virtual void deleteUniform(UniformHandle handle) = 0;
	virtual void createTexture(TextureHandle handle, uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, uint32_t count, uint8_t mips, uint8_t samples) = 0;
	virtual void createTexture(TextureHandle handle, void* mem, size_t size, ImageInfo& info) = 0;
	virtual void deleteTexture(TextureHandle handle) = 0;
	virtual void updateTexture(TextureHandle handle, void* mem) = 0;
	virtual void readTexture(TextureHandle handle, void* mem) = 0;
	virtual void generateMips(TextureHandle handle) = 0;
	virtual void createSampler(SamplerHandle handle, uint32_t flags, uint8_t anisotrophic) = 0;
	virtual void deleteSampler(SamplerHandle handle) = 0;
	virtual void createFramebuffer(FramebufferHandle handle, TextureHandle* texHandles, uint32_t num, uint32_t index) = 0;
	virtual void deleteFramebuffer(FramebufferHandle handle) = 0;
    };

    class Context
    {
    public:

	DCFX_API Context(void* window, dcutil::AllocatorI* allocator = NULL, CallbackI* callback = NULL);
	DCFX_API ~Context();
		
	DCFX_API void setView(uint16_t index, const glm::mat4& view);
	DCFX_API void setProj(uint16_t index, const glm::mat4& proj);

	DCFX_API void* frameAlloc(size_t size);
	
	// Call to advance a frame
	DCFX_API uint64_t frame();
			
	// Resource creation
	DCFX_API BufferHandle createVertexBuffer(void* mem, size_t size, const VertexDecl& decl); // TODO: Pass in stride
	DCFX_API BufferHandle createBuffer(void* mem, size_t size, BufferType type); // TODO: Pass in stride
	DCFX_API void deleteBuffer(BufferHandle handle);
	DCFX_API void updateBuffer(BufferHandle handle, size_t offset, size_t size, void* mem);
	DCFX_API void readBuffer(BufferHandle handle, size_t offset, size_t size, void* mem);
	DCFX_API ShaderHandle createShader(void* mem, size_t size, ShaderType type);
	DCFX_API void deleteShader(ShaderHandle handle);
	DCFX_API ProgramHandle createProgram(ProgramDesc desc, bool freeShaders = true);
	DCFX_API void deleteProgram(ProgramHandle handle);
	DCFX_API UniformHandle createUniform(const char* name, UniformType type, uint8_t num);
	DCFX_API void deleteUniform(UniformHandle handle);
	DCFX_API TextureHandle createTexture(uint32_t width, uint32_t height, TextureFormat format, uint32_t depth = 1, uint32_t count = 1, uint8_t mips = 1, uint8_t samples = 1);
	DCFX_API TextureHandle createTexture(void* mem, size_t size, ImageInfo& info);	
	DCFX_API void deleteTexture(TextureHandle handle);
	DCFX_API void updateTexture(TextureHandle handle, void* mem);
	DCFX_API void readTexture(TextureHandle handle, void* mem);
	DCFX_API void generateMips(TextureHandle handle);
	DCFX_API void generateMips(TextureHandle handle, uint16_t view);
	DCFX_API SamplerHandle createSampler(uint32_t state, uint8_t anisotrophic = 0);
	DCFX_API void deleteSampler(SamplerHandle handle);
	DCFX_API FramebufferHandle createFramebuffer(TextureHandle* handles, uint32_t num, uint32_t index = 0);
	DCFX_API void deleteFramebuffer(FramebufferHandle handle);			
	DCFX_API void setUniform(UniformHandle handle, const void* mem, size_t size);
	DCFX_API void setTexture(UniformHandle uniform, TextureHandle texture, uint32_t state);
	DCFX_API void setTexture(UniformHandle uniform, TextureHandle texture, SamplerHandle sampler);	
	DCFX_API void setImage(UniformHandle sampler, TextureHandle texture, ImageBind::Access access = ImageBind::Access::READWRITE, uint8_t mip = 0, bool layered = false, uint8_t layer = 0);
	DCFX_API void setBuffer(uint16_t binding, BufferHandle buffer);	
	DCFX_API void setFramebuffer(uint16_t index, FramebufferHandle framebuffer);
	DCFX_API void setViewport(uint16_t index, const glm::ivec4& viewport);
	DCFX_API void setClearColor(uint16_t index, const glm::vec4& clearColor);
	DCFX_API void setVertexBuffer(BufferHandle handle, uint32_t baseVertex = 0, uint32_t count = 0);
	DCFX_API void setIndexBuffer(BufferHandle handle, uint32_t indexOffset = 0, uint32_t count = 0);
	DCFX_API void setInstanceBuffer(BufferHandle handle, uint32_t numInstances, uint16_t stride, uint32_t offset = 0);	
	DCFX_API void setProgram(ProgramHandle handle);
	DCFX_API void setTransform(const glm::mat4& transform);
	DCFX_API void setState(uint32_t state);
	DCFX_API void setPrimitiveMode(uint32_t mode);
	DCFX_API void blitFramebuffer(uint16_t view, FramebufferHandle from, FramebufferHandle to);
	DCFX_API void submit(uint16_t view, uint32_t flag = 1);
	DCFX_API void dispatch(uint16_t view, ProgramHandle handle, uint16_t numX, uint16_t numY, uint16_t numZ); 

	DCFX_API bool isInited() { return m_renderInited; }

    private:
	void freeHandles();
	
	// Swaps the submit datastructure with the render one
	void swap();

	void execCommands(CommandBuffer* buffer);

	static int32_t renderThread(Context* ctx);

	// Called by the render thread
	bool renderFrame();

	VertexDeclHandle findVertexDecl(const VertexDecl& decl);
	VertexDeclHandle createVertexDecl(const VertexDecl& decl);

	bool m_exit;
	std::atomic_bool m_renderInited;
	bool m_useCrtAllocator;
	bool m_useCallbackStub;

	std::thread m_renderThread;
	dcutil::Semaphore m_renderSem;
	dcutil::Semaphore m_gameSem;

	RenderContextI* m_context;
	Frame m_frames[2];
	Frame* m_render;
	Frame* m_submit;
	void* m_renderTempMem;
	void* m_submitTempMem;
	uint64_t m_frameCount;

	dcutil::HandleAllocator<DCFX_MAX_UNIFORMS> m_uniformHandles;
	dcutil::HandleAllocator<DCFX_MAX_VERTEX_DECLS> m_vertexDeclHandles;	
	dcutil::HandleAllocator<DCFX_MAX_BUFFERS> m_bufferHandles;
	dcutil::HandleAllocator<DCFX_MAX_SHADERS> m_shaderHandles;
	dcutil::HandleAllocator<DCFX_MAX_PROGRAMS> m_programHandles;
	dcutil::HandleAllocator<DCFX_MAX_TEXTURES> m_textureHandles;
	dcutil::HandleAllocator<DCFX_MAX_SAMPLERS> m_samplerHandles;
	dcutil::HandleAllocator<DCFX_MAX_FRAMEBUFFERS> m_framebufferHandles;

	typedef tinystl::unordered_map<uint32_t, VertexDeclHandle> VertexDeclMap;
	VertexDeclMap* m_vertexDeclMap;
    };
}

