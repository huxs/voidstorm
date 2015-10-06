#version 330

uniform mat4 g_view;

in vec3 in_position;
in vec4 in_color;

out vec4 frag_color;

void main()
{
    gl_Position = g_view * vec4(in_position, 1.0);

    frag_color = in_color;
}
