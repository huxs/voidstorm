#version 330

uniform sampler2D Scene;

in vec2 fs_texCoord;

 out vec4 fragmentColor;

void main()
{
    fragmentColor = texture(Scene, fs_texCoord);
}
