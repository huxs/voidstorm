#version 330

uniform mat4 g_View;

in vec3 in_position;
in vec4 in_color;

out vec4 frag_color;

void main()
{
    gl_Position = g_View * vec4(in_position, 1.0);

    frag_color = in_color;
}
