glm::vec2 PolygonShape::computeCentroid()
{
    assert(count >= 3);

    glm::vec2 c(0,0);
    float area = 0;

    glm::vec2 ref(0,0);
    
    static const float Inv3 = 1.0f / 3.0f;

    for(int i = 0; i < count; ++i)
    {
	glm::vec2 p0 = ref;
	glm::vec2 p1 = vertices[i];
	glm::vec2 p2 = i + 1 < count ? vertices[i+1] : vertices[0];

	glm::vec2 e0 = p1 - p0;
	glm::vec2 e1 = p2 - p1;

	/* NOTE (daniel):Returns the magnitude of the Z-vector after cross with e0,e1
	   treating the vectors on a plane in 3D. Thus the result is a scalar */       
	float D = e0.x * e1.y - e0.y * e1.x;

	float triangleArea = 0.5f * D;
	area += triangleArea;

	c += triangleArea * Inv3 * (p0 + p1 + p2);
    }

    assert(area > VOIDSTORM_EPSILON);
    c *= 1.0f / area;
    return c;
}

AABB PolygonShape::computeAABB()
{
    AABB result;
    
    result.lower = vertices[0];
    result.upper = result.lower;

    for(int i = 1; i < count; ++i)
    {
	glm::vec2 v = vertices[i];
	result.lower = glm::min(result.lower, v);
	result.upper = glm::max(result.upper, v);
    }

    return result;
}

void PolygonShape::set(glm::vec2* _vertices, uint16_t _count)
{
    assert(_count >= 3 && _count <= VOIDSTORM_MAX_POLYGON_VERTICES);

    // TODO (daniel): Remove duplicates
    glm::vec2* ps = _vertices;
    int n = (int)_count;

    // Create the convex hull using the Gift wrapping algorithm
    // http://en.wikipedia.org/wiki/Gift_wrapping_algorithm

    // Find the right most point on the hull
    int i0 = 0;
    float x0 = ps[0].x;
    for (int i = 1; i < n; ++i)
    {
	float x = ps[i].x;
	if (x > x0 || (x == x0 && ps[i].y < ps[i0].y))
	{
	    i0 = i;
	    x0 = x;
	}
    }

    int hull[VOIDSTORM_MAX_POLYGON_VERTICES];
    int m = 0;
    int ih = i0;

    for (;;)
    {
	hull[m] = ih;

	int ie = 0;
	for (int j = 1; j < n; ++j)
	{
	    if (ie == ih)
	    {
		ie = j;
		continue;
	    }

	    glm::vec2 r = ps[ie] - ps[hull[m]];
	    glm::vec2 v = ps[j] - ps[hull[m]];

	    float c = r.x * v.y - r.y * v.x;	    
	    if (c < 0.0f)
	    {
		ie = j;
	    }

	    float lenv = glm::length(v);
	    float lenr = glm::length(r);

	    // Collinearity check
	    if (c == 0.0f && lenv*lenv > lenr*lenr)
	    {
		ie = j;
	    }
	}

	++m;
	ih = ie;

	if (ie == i0)
	{
	    break;
	}
    }

    count = m;
    
    // Copy vertices
    for (int i = 0; i < m; ++i)
    {
	vertices[i] = ps[hull[i]];
    }

    // Compute normals. Ensure the edges have non-zero length
    for (int i = 0; i < m; ++i)
    {
	int i1 = i;
	int i2 = i + 1 < m ? i + 1 : 0;
	glm::vec2 edge = vertices[i2] - vertices[i1];
	float lenedge = glm::length(edge);
	assert(lenedge*lenedge > 0.00001f * 0.00001f);

	// Cross product between vector and scalar
	normals[i] = glm::vec2(1.0f * edge.y, -1.0f * edge.x);
	normals[i] = glm::normalize(normals[i]);
    }

    centroid = computeCentroid();
}

void PolygonShape::setAsBox(float width, float height)
{
    count = 4;
    
    vertices[0] = glm::vec2(-width, -height);
    vertices[1] = glm::vec2(width, -height);
    vertices[2] = glm::vec2(width, height);
    vertices[3] = glm::vec2(-width, height);
    
    normals[0] = glm::vec2(0, -1);
    normals[1] = glm::vec2(1, 0);
    normals[2] = glm::vec2(0, 1);
    normals[3] = glm::vec2(-1, 0);
    
    centroid = computeCentroid();
}

// TODO (daniel): Add support for rotated boxes
void PolygonShape::setAsBox(float width, float height, const glm::vec2& center)
{
    count = 4;
    
    vertices[0] = glm::vec2(-width, -height) + center;
    vertices[1] = glm::vec2(width, -height) + center;
    vertices[2] = glm::vec2(width, height) + center;
    vertices[3] = glm::vec2(-width, height) + center;
    
    normals[0] = glm::vec2(0, -1);
    normals[1] = glm::vec2(1, 0);
    normals[2] = glm::vec2(0, 1);
    normals[3] = glm::vec2(-1, 0);
    
    centroid = center;
}


