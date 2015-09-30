#version 330

uniform mat4 g_view;

in vec3 in_position;
in vec2 in_texCoord;
in vec4 in_color;

out vec2 fs_texCoord;
out vec4 fs_color;

void main()
{
    fs_texCoord = in_texCoord;
    fs_color = in_color;

    gl_Position = g_view * vec4(in_position, 1.0f);
}
