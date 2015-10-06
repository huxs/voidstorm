#version 330

uniform sampler2D Scene;
uniform float Exposure;

in vec2 fs_texCoord;

layout(location = 0) out vec4 fragmentColor;

void main()
{
    vec3 hdr = texture(Scene, fs_texCoord).rgb;

    hdr *= pow(2.0, Exposure);

    vec3 color = hdr / (1+hdr);

    fragmentColor = vec4(color, 1.0);
}