#pragma once

#include "dcfx_helper.h"
#include <stdint.h>

#define DCFX_MAX_ATTRIBUTES 16

namespace dcfx
{
    enum class AttributeType
    {
	FLOAT,
	INT,
	SHORT,
	BYTE
    };

    static const uint8_t s_attrTypeSizes[3] =
    { 4, 4, 1 };

    class VertexDecl
    {
    public:
	DCFX_API VertexDecl();
	DCFX_API void begin();
	DCFX_API void add(uint8_t location, uint8_t num, AttributeType type, bool normalized);
	DCFX_API void decode(uint8_t location, uint8_t& num, AttributeType& type, bool& normalized) const;
	DCFX_API void end();
		
	uint8_t m_numAttribs;
	uint8_t m_attributes[DCFX_MAX_ATTRIBUTES];
	uint8_t m_offsets[DCFX_MAX_ATTRIBUTES];
	uint32_t m_stride;
	uint32_t m_hash;
    };
}


