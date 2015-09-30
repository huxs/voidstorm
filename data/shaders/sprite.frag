#version 330

uniform sampler2D g_colorMap;

in vec2 fs_texCoord;
in vec4 fs_color;

layout(location = 0) out vec4 fragmentColor;

void main()
{
    fragmentColor = texture(g_colorMap, fs_texCoord) * fs_color;
}
