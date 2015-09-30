#version 330

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D ColorMap;

uniform float TapSize;
uniform float Sigma;

in vec2 fs_texCoord;

layout(location = 0) out vec4 fragmentColor;

//const float TapSize = 1.3;
//const float Sigma = 1.7;

float gauss(float x)
{
	return (1.0 / (Sigma*sqrt(2.0*M_PI))) * exp(-(pow(x,2.0) / (2.0*pow(Sigma,2.0))));
}
 
void main()
{
    float size = 1.0/textureSize(ColorMap, 0).x * TapSize;
    vec4 sum = vec4(0.0);

	float y0 = gauss(0);
	float y1 = gauss(1.0);
	float y2 = gauss(2.0);
	float y3 = gauss(3.0);
	float y4 = gauss(4.0);
	float y5 = gauss(5.0);

	sum += texture(ColorMap, vec2(fs_texCoord.x - 5.0*size, fs_texCoord.y)) * y5;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 4.0*size, fs_texCoord.y)) * y4;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 3.0*size, fs_texCoord.y)) * y3;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 2.0*size, fs_texCoord.y)) * y2;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 1.0*size, fs_texCoord.y)) * y1;
    sum += texture(ColorMap, vec2(fs_texCoord.x, fs_texCoord.y)) * y0;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 1.0*size, fs_texCoord.y)) * y1;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 2.0*size, fs_texCoord.y)) * y2;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 3.0*size, fs_texCoord.y)) * y3;
	sum += texture(ColorMap, vec2(fs_texCoord.x + 4.0*size, fs_texCoord.y)) * y4;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 5.0*size, fs_texCoord.y)) * y5;
	//sum = vec4(0.0);
	
/*
    //5tap Sigma: 1.3
    sum += texture(ColorMap, vec2(texCoord.x - 2.0*size, texCoord.y)) * 0.13;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 1.0*size, fs_texCoord.y)) * 0.23;
    sum += texture(ColorMap, vec2(fs_texCoord.x, fs_texCoord.y)) * 0.28;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 1.0*size, fs_texCoord.y)) * 0.23;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 2.0*size, fs_texCoord.y)) * 0.13;

    // 7tap Sigma 1.7
    sum += texture(ColorMap, vec2(fs_texCoord.x - 3.0*size, fs_texCoord.y)) * 0.07;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 2.0*size, fs_texCoord.y)) * 0.13;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 1.0*size, fs_texCoord.y)) * 0.19;
    sum += texture(ColorMap, vec2(fs_texCoord.x, fs_texCoord.y)) * 0.21;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 1.0*size, fs_texCoord.y)) * 0.19;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 2.0*size, fs_texCoord.y)) * 0.13;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 3.0*size, fs_texCoord.y)) * 0.07;

    // 7tap Sigma 1
    sum += texture(ColorMap, vec2(fs_texCoord.x - 3.0*size, fs_texCoord.y)) * 0.016;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 2.0*size, fs_texCoord.y)) * 0.083;
    sum += texture(ColorMap, vec2(fs_texCoord.x - 1.0*size, fs_texCoord.y)) * 0.23;
    sum += texture(ColorMap, vec2(fs_texCoord.x, fs_texCoord.y)) * 0.33;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 1.0*size, fs_texCoord.y)) * 0.23;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 2.0*size, fs_texCoord.y)) * 0.083;
    sum += texture(ColorMap, vec2(fs_texCoord.x + 3.0*size, fs_texCoord.y)) * 0.016;
*/
    fragmentColor = sum;
}
