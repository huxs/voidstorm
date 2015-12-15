#pragma once

#include <glm/glm.hpp>

namespace dcutil
{
    static glm::vec4 toRGBFromHSV(const glm::vec4& hsv)
    {
	// H [0, 360] S and V [0.0, 1.0].
	float h = hsv.x;
	float s = hsv.y;
	float v = hsv.z;

	int i = (int)floor(h/60.0f) % 6;
	float f = h/60.0f - floor(h/60.0f);
	float p = v * (float)(1 - s);
	float q = v * (float)(1 - s * f);
	float t = v * (float)(1 - (1 - f) * s);

	glm::vec4 rgb = glm::vec4(0,0,0,hsv.w);
    
	switch (i) {
	case 0:
	    rgb.x = v;
	    rgb.y = t;
	    rgb.z = p;
	    break;
	case 1:
	    rgb.x = q;
	    rgb.y = v;
	    rgb.z = p;
	    break;
	case 2:
	    rgb.x = p;
	    rgb.y = v;
	    rgb.z = t;
	    break;
	case 3:
	    rgb.x = p;
	    rgb.y = q;
	    rgb.z = v;
	    break;
	case 4:
	    rgb.x = t;
	    rgb.y = p;
	    rgb.z = v;
	    break;
	case 5:
	    rgb.x = v;
	    rgb.y = p;
	    rgb.z = q;
	    break;
	}

	return rgb;
    }

    static glm::vec4 toHSVFromRGB(const glm::vec4& rgb)
    {
	float r = rgb.x;
	float g = rgb.y;
	float b = rgb.z;

	float minRGB = glm::min(r, glm::min(g, b));
	float maxRGB = glm::max(r, glm::max(g, b));

	glm::vec4 hsv = glm::vec4(0,0,0,rgb.w);
    
	if(minRGB == maxRGB)
	{
	    hsv.z = minRGB;
	    return hsv;
	}

	float d = (r == minRGB) ? g-b : ((b == minRGB) ? r-g : b-r);
	int h = (r == minRGB) ? 3 : ((b == minRGB) ? 1 : 5);

	hsv.x = 60.0f * (h - d / (maxRGB - minRGB));
	hsv.y = (maxRGB - minRGB) / maxRGB;
	hsv.z = maxRGB;

	return hsv;
    }
}
