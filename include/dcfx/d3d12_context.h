#pragma once

#include "context.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

// NOTE(daniel): This is for UpdateSubresources
#include <d3dx12.h>

namespace dcfx
{
    struct D3D12IndirectCommand
    {
	D3D12_GPU_VIRTUAL_ADDRESS m_cbv;
	D3D12_DRAW_ARGUMENTS m_drawArguments;
    };

    struct D3D12Batch
    {
	void begin();
	void flush();
	void draw(const DrawCall& draw, D3D12_GPU_VIRTUAL_ADDRESS cb);
	void end();

	D3D12IndirectCommand m_cmds[DCFX_MAX_DRAW_CALLS];
	uint16_t m_cmdIndex;
    };
    
    struct D3D12Shader
    {
	void create(void* mem, size_t size, ShaderType type);
	void destroy();

	ID3DBlob* m_shaderBlob;
    };

    struct D3D12Program
    {
	void create(D3D12Shader* vs, D3D12Shader* ps);
	void destroy();

	ID3D12PipelineState* m_pso;

	ID3D12Resource* m_cb;
	void* m_cbData;
    };
    
    struct D3D12Buffer
    {
	void create(void* mem, size_t size, size_t stride, BufferType type);
	void destroy();


	ID3D12Resource* m_resource;
	D3D12_VERTEX_BUFFER_VIEW m_view;
    };

    struct D3D12Texture
    {
//	void create(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format);
//	void create(uint32_t width, uint32_t height, TextureFormat format);
	void create(ImageInfo& info, void* data, size_t size);
	void destroy();
//	void update(void* mem);

	ID3D12Resource* m_texture;
	//TODO: Identifer for a texture.
    };
    
    struct D3D12RenderContext : public RenderContextI
    {
	D3D12RenderContext();
	~D3D12RenderContext();

	void flip();
	void render(Frame* frame);

	// resource creation.
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
	void createTexture(TextureHandle handle, uint32_t width, uint32_t height, uint32_t depth, TextureFormat format);
	void createTexture(TextureHandle handle, void* mem, size_t size, ImageInfo& info);
	void deleteTexture(TextureHandle handle);
	void updateTexture(TextureHandle handle, void* mem);
	void createFramebuffer(FramebufferHandle handle, TextureHandle* texHandles, uint32_t num);
	void deleteFramebuffer(FramebufferHandle handle);

	void updateUniformData(UniformBuffer& buffer, size_t begin, size_t end);
	void commit();
	
	ID3D12Device* m_device;
	ID3D12CommandQueue* m_commandQueue;
	IDXGISwapChain3* m_swapChain;
	ID3D12DescriptorHeap* m_rtvHeap;
	ID3D12DescriptorHeap* m_cbvHeap;
	UINT m_rtvDescriptorSize;
	UINT m_cbvSrvDescriptorSize;
	ID3D12Resource* m_renderTargets[2];
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12RootSignature* m_rootSignature;
	ID3D12CommandSignature* m_commandSignature;

	ID3D12Resource* m_commandBuffer;
	void* m_commandData;
	
	uint16_t m_backbufferIndex;
	HANDLE m_event;

	ID3D12Fence* m_fence;
	uint64_t m_fenceValue;
	
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	D3D12Buffer m_buffers[DCFX_MAX_BUFFERS];
	D3D12Shader m_shaders[DCFX_MAX_SHADERS];
	D3D12Program m_programs[DCFX_MAX_PROGRAMS];
	D3D12Texture m_textures[DCFX_MAX_TEXTURES];

	void* m_unifomData[DCFX_MAX_UNIFORMS];

	D3D12Batch m_batch;

	// TODO: Use the id instead to find a hole in the heap!
	UINT m_textureOffset;
	
    };
}
