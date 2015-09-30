#version 330

in vec3 in_position;
in vec2 in_texCoord;

out vec2 fs_texCoord;

void main()
{
	fs_texCoord = in_texCoord;

	gl_Position = vec4(in_position, 1.0f);
}
