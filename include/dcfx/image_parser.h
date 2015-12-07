#pragma once

#include "ddraw.h"

#include <dcutil/memoryreader.h>
#include <dcutil/macros.h>
#include <dcutil/print.h>

#include <stdint.h>

#define DDS_MAGIC MAKEFOURCC('D', 'D', 'S', ' ')
#define DDS_DX10 MAKEFOURCC('D', 'X', '1', '0')

#define D3DFMT_A8R8G8B8 65
#define D3DFMT_A32R32G32B32 116

#define DDPF_FOURCC 0x00000004

// NOTE: dwCaps
#define DDSCAPS_COMPLEX 0x8
#define DDSCAPS_MIPMAP 0x400000
#define DDSCAPS_TEXTURE 0x1000

// NOTE: dwCaps2
#define DDSCAPS2_CUBEMAP 0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000

#define DDS_CUBEMAP_ALLFACES (DDSCAPS2_CUBEMAP_POSITIVEX|DDSCAPS2_CUBEMAP_NEGATIVEX \
			      |DDSCAPS2_CUBEMAP_POSITIVEY|DDSCAPS2_CUBEMAP_NEGATIVEY \
			      |DDSCAPS2_CUBEMAP_POSITIVEZ|DDSCAPS2_CUBEMAP_NEGATIVEZ)
namespace dcfx
{
    enum TextureFormat
    {
	BC1 = 0,
	BC3,
	BC5,
	RGBA8,
	RGBA32F,
	RG16F,
	RG32F,
	R32F,
	D32F,
	D32FS8,
	D24S8
    };

    struct ImageInfo
    {
	ImageInfo()
		:
		m_isSRGB(true)
	    {}
	
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pitch;
	TextureFormat m_format;
	uint32_t m_offset;
	uint32_t m_numMips;
	uint32_t m_depth;
	bool m_isCompressed;
	bool m_isCubeMap;
	bool m_isSRGB;
    };

    struct TranslateDDSFormat
    {
	uint32_t m_ddsFormat;
	TextureFormat m_textureFormat;
    };

    struct DDSFormatInfo
    {
	uint32_t m_bitsPerPixel;
	uint32_t m_blockSize;
	uint32_t m_blockWidth;
	uint32_t m_blockHeight;
    };

    static TranslateDDSFormat s_translateDDSFormat[] =
    {
	{ FOURCC_DXT1, TextureFormat::BC1 },
	{ FOURCC_DXT3, TextureFormat::BC3 },
	{ FOURCC_DXT5, TextureFormat::BC5 },
	{ D3DFMT_A8R8G8B8, TextureFormat::RGBA8 },
	{ D3DFMT_A32R32G32B32, TextureFormat::RGBA32F }
    };

    // NOTE: BPP, BlkSize, BlkWidth, BlkHeight
    static DDSFormatInfo s_ddsFormatInfo[] =
    {
	{ 4, 8, 4, 4 },
	{ 8, 16, 4, 4 },
	{ 8, 16, 4, 4 },
	{ 32, 4, 1, 1 },
	{ 128, 4, 1, 1}
    };
    
    bool imageDDSParse(dcutil::MemoryReader* reader, ImageInfo& imageInfo);
    bool imagePNGParse(dcutil::MemoryReader* reader, ImageInfo& imageinfo);
    bool imageParse(void* mem, size_t size, ImageInfo& imageInfo);
}


