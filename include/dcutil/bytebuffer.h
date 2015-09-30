#pragma once 

#include <stdint.h>
#include <assert.h>
#include <string.h>

namespace dcutil
{
    static const uint8_t CmdEnd = UINT8_MAX;

    template<size_t sizeT>
    class ByteBuffer
    {
    public:

	ByteBuffer()
		: m_pos(0), m_size(sizeT)
	    {
		memset(m_buffer, 0, sizeT);
	    }

	void read(void* data, size_t size)
	    {
		assert(m_pos < m_size);
		memcpy(data, &m_buffer[m_pos], size);
		m_pos += size;
	    }

	template<typename T>
	void read(T& data)
	    {
		read(&data, sizeof(T));
	    }

	void* skip(size_t size)
	    {
		void* data = &m_buffer[m_pos];
		m_pos += size;
		return data;
	    }

	void write(const void* data, size_t size)
	    {
		assert(m_pos < m_size);
		memcpy(&m_buffer[m_pos], data, size);
		m_pos += size;
	    }

	template<typename T>
	void write(const T& data)
	    {
		write(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
	    }

	void start()
	    {
		m_pos = 0;
		m_size = sizeT;
	    }

	void finnish()
	    {
		m_size = m_pos;
		m_pos = 0;
	    }

	size_t m_pos;

    private:
	size_t m_size;
	uint8_t m_buffer[sizeT];
    };
}
