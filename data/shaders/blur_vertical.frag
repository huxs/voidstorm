#version 330

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D Scene;
uniform float Sigma;
uniform float TapSize;

in vec2 fs_texCoord;

out vec4 fragmentColor;

float gauss(float x)
{
    return (1.0 / (Sigma*sqrt(2.0*M_PI))) * exp(-(pow(x,2.0) / (2.0*pow(Sigma,2.0))));
}

void main()
{
    float size = 1.0 / textureSize(Scene, 0).y * TapSize;
    vec4 sum = vec4(0.0);
    float weightSum = 0.0;
    
    int i = 0;
    for(i = -10; i <= 10; ++i)
    {
	float y = gauss(i);
	weightSum += y;
	sum += texture(Scene, vec2(fs_texCoord.x, fs_texCoord.y + i * size)) * y;
    }

    sum /= weightSum;
    
    fragmentColor = sum;
}

