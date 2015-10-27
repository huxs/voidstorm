#pragma once

#include "context.h"
#include "image_parser.h"

/* TODO:
   Implement 1D textures.
   Implement support for geometry/tesselation.
   Implement stenciling.
   Implement support for alpha testing.
   Scissor test.   
*/

#ifdef WINDOWS
#include "opengl_wgl.h"
#else
#error("OpenGL context creation not implemented for this system.")
#endif

#define DCFX_GL_MAJOR_VERSION 4
#define DCFX_GL_MINOR_VERSION 4
#define DCFX_GL_SAMPLE_BUFFERS 1
#define DCFX_GL_SAMPLES 8
#define DCFX_GL_PROGRAM_UNIFORM_BUFFER_SIZE 1024
#define DCFX_GL_PROGRAM_PREDFINED_UNIFORM_COUNT 8

namespace dcfx
{
    typedef dcutil::ByteBuffer<DCFX_GL_PROGRAM_UNIFORM_BUFFER_SIZE> ProgramBuffer;

    static const GLenum s_bufferMap[] = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER };
    static const GLenum s_shaderMap[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER };
    static const GLenum s_typeMap[] = { GL_FLOAT, GL_SHORT, GL_INT, GL_BYTE };

    static const GLenum s_textureFormatMap[] =
    {
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
    };

    struct GLRenderContext;

    struct GLBuffer
    {
	void create(void* mem, size_t size, BufferType type);
	void destroy();
	void update(size_t offset, size_t size, void* mem);
	void read(size_t offset, size_t size, void* mem);

	GLuint m_handle;
	GLenum m_type;
    };

    struct GLShader
    {
	void create(void* mem, ShaderType type);
	void destroy();

	GLuint m_handle;
    };

    struct GLPredfinedUniform
    {
	GLint m_loc;
	PredefinedUniformType m_type;
    };

    struct GLProgram
    {
	void create(GLRenderContext* context, GLShader* vert, GLShader* frag);
	void destroy();
	void bindAttributes(const VertexDecl& decl);
	void bindInstanceData(const VertexDecl& decl, uint16_t stride);
		
	GLuint m_handle;
	ProgramBuffer m_uniforms;
	GLPredfinedUniform m_predefined[DCFX_GL_PROGRAM_PREDFINED_UNIFORM_COUNT];
	uint16_t m_samplers[DCFX_MAX_TEXTURE_SAMPLERS];
	uint8_t m_predefinedCount;
	uint8_t m_samplerCount;
	uint8_t m_activeAttributes;
    };

    struct GLTexture
    {
	void setFormat(TextureFormat format);
	
	void create(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format);
	void createArray(uint32_t width, uint32_t height, TextureFormat format, uint32_t count);
	void create(uint32_t width, uint32_t height, TextureFormat format);
	void create(ImageInfo& info, void* data, size_t size);
	void destroy();
	void update(void* mem);

	GLuint m_handle;
	GLenum m_target;
	
	GLenum m_type; // Texel Type
	GLenum m_format; // Texel Format
	GLenum m_internal; // GL Storage format
	
	glm::ivec3 m_size;
        
/*
  TODO (daniel):
  Anistrophic level
  Base mip level
  Max mip level
  Comp value
  Border color
*/

	uint32_t m_flags;
    };

    struct GLFramebuffer
    {
	void create(GLTexture* textures, TextureHandle* handles, uint32_t num, uint32_t index);
	void destroy();
			
	GLuint m_handle;
    };
		
    struct GLRenderContext : public RenderContextI
    {
	GLRenderContext();
	~GLRenderContext();
	void flip();
	void render(Frame* frame);
			
	void createVertexDecl(VertexDeclHandle handle, const VertexDecl& decl);
	void deleteVertexDecl(VertexDeclHandle handle);
	void createVertexBuffer(BufferHandle handle, void* mem, size_t size, VertexDeclHandle decl);
	void createBuffer(BufferHandle handle, void* mem, size_t size, BufferType type);
	void deleteBuffer(BufferHandle handle);
	void updateBuffer(BufferHandle handle, size_t offset, size_t size, void* mem);
	void readBuffer(BufferHandle handle, size_t offset, size_t size, void* mem);
	void createShader(ShaderHandle handle, void* mem, size_t size, ShaderType type);
	void deleteShader(ShaderHandle handle);
	void createProgram(ProgramHandle handle, ProgramDesc desc);
	void deleteProgram(ProgramHandle handle);
	void createUniform(UniformHandle handle, const char* name, UniformType type, uint8_t num);
	void deleteUniform(UniformHandle handle);
	void createTexture(TextureHandle handle, uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, uint32_t count);
	void createTexture(TextureHandle handle, void* mem, size_t size, ImageInfo& info);
	void deleteTexture(TextureHandle handle);
	void updateTexture(TextureHandle handle, void* mem);
	void createFramebuffer(FramebufferHandle handle, TextureHandle* texHandles, uint32_t num, uint32_t index);
	void deleteFramebuffer(FramebufferHandle handle);

	void updateUniforms(UniformBuffer& buffer, size_t begin, size_t end);
	void commit(ProgramBuffer& buffer);

	GLContext m_context;	
	GLuint m_timeElapsed;

	void* m_unifomData[DCFX_MAX_UNIFORMS];
	tinystl::unordered_map<uint32_t, UniformDesc> m_uniformDesc;

	VertexDecl m_vertexDecls[DCFX_MAX_VERTEX_DECLS];
	GLProgram m_programs[DCFX_MAX_PROGRAMS];
	GLShader m_shaders[DCFX_MAX_SHADERS];
	GLBuffer m_buffers[DCFX_MAX_BUFFERS];
	tinystl::unordered_map<uint16_t, VertexDeclHandle> m_bufferToDecls;

	GLTexture m_textures[DCFX_MAX_TEXTURES];
	GLFramebuffer m_framebuffers[DCFX_MAX_FRAMEBUFFERS];
    };
}


