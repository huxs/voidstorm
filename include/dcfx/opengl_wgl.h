#pragma once

#define GLEW_STATIC
#include <gl/glew.h>
#include <wgl/wglext.h>
#include <stdint.h>

struct GLContext
{
    void create(void* hwnd);
    void destroy();
    void swap();
    
    int32_t m_contextAttrs[9];
    PIXELFORMATDESCRIPTOR m_pfd;
    int m_pixelFormat;
    HWND m_hwnd;
    HGLRC m_context;
    HDC m_hdc;
};
