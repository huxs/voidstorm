#version 330

uniform mat4 g_view;

in vec3 in_position;

void main()
{
    gl_Position = g_view * vec4(in_position, 1.0);
}
