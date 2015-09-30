#version 330

uniform sampler2D ColorMap;

in vec2 fs_texCoord;

layout(location = 0) out vec4 fragmentColor;

void main()
{
    fragmentColor = texture(ColorMap, fs_texCoord);
}