#pragma once

#define DCFX_MAX_VIEWS 255
#define DCFX_MAX_DRAW_CALLS (1<<12) // 4096
#define DCFX_MAX_TEXTURES_TO_MIP_PER_VIEW 5

#define DCFX_MAX_COMMAND_BUFFER_SIZE (64<<10) // Frame command buffer 65kb
#define DCFX_MAX_UNIFORM_BUFFER_SIZE (64<<10) // Frame uniform buffer 65kb
#define DCFX_MATRIX_CACHE_COUNT (1<<12) // 4096 unique matrices

#define DCFX_MAX_IMAGE_UNITS 16 // GL_MAX_IMAGE_UNITS
#define DCFX_MAX_TEXTURE_SAMPLERS 16 // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
#define DCFX_MAX_COMPUTE_BUFFER_BINDINGS 8 // GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS

#define DCFX_MAX_VERTEX_DECLS 64
#define DCFX_MAX_BUFFERS (1<<12) // 4096
#define DCFX_MAX_SHADERS 128
#define DCFX_MAX_PROGRAMS 64
#define DCFX_MAX_UNIFORMS 128
#define DCFX_MAX_TEXTURES 128
#define DCFX_MAX_SAMPLERS 64
#define DCFX_MAX_FRAMEBUFFERS 64

#define DCFX_MULTITHREADED 1
#define DCFX_FRAME_ALLOCATOR_SIZE (32<<22)
