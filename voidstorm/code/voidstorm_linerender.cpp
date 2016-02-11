LineRenderer::LineRenderer(dcfx::Context* _renderCtx)
	: vertexCount(0), renderCtx(_renderCtx)
{
    vertices = (LineVertex*)g_heapAllocator->alloc(VOIDSTORM_DEBUG_NUM_LINES * sizeof(LineVertex) * 2);
	
    dcfx::ProgramDesc desc;
    desc.m_vert = loadShader(renderCtx, "line.vert", dcfx::ShaderType::VERTEX);
    desc.m_frag = loadShader(renderCtx, "line.frag", dcfx::ShaderType::FRAGMENT);
    program= renderCtx->createProgram(desc);

    vertexDecl.begin();
    vertexDecl.add(0, 3, dcfx::AttributeType::FLOAT, false);
    vertexDecl.add(1, 4, dcfx::AttributeType::FLOAT, false);
    vertexDecl.end();

    vertexBuffer = renderCtx->createVertexBuffer(
	nullptr,
	VOIDSTORM_DEBUG_NUM_LINES * sizeof(LineVertex) * 2,
	vertexDecl);
}

LineRenderer::~LineRenderer()
{
    g_heapAllocator->free(vertices);

    renderCtx->deleteProgram(program);
    renderCtx->deleteBuffer(vertexBuffer);
}

void LineRenderer::add(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color)
{
    assert(vertexCount + 1 < VOIDSTORM_DEBUG_NUM_LINES * sizeof(LineVertex) * 2);
    
    vertices[vertexCount++] = { glm::vec3(from.x, from.y, 0), color };
    vertices[vertexCount++] = { glm::vec3(to.x, to.y, 0), color };
}

void LineRenderer::dispatch(int view)
{
    renderCtx->updateBuffer(vertexBuffer, 0, vertexCount * sizeof(LineVertex), &vertices[0]);
	
    renderCtx->setState(DCFX_DEFAULT_STATE);
    renderCtx->setPrimitiveMode(DCFX_STATE_PT_LINES);
    renderCtx->setIndexBuffer({ dcfx::InvalidHandle }, 0, 0);
    renderCtx->setVertexBuffer(vertexBuffer, 0, (uint32_t)vertexCount);
    renderCtx->setProgram(program);
    renderCtx->submit(view);
}

void LineRenderer::reset()
{
   vertexCount = 0;
}

void LineRenderer::drawCircle(const glm::vec2& pos, float radius, const glm::vec4& color)
{
    static const int NumberOfPoints = 12;
    static const float DeltaAngle = (2 * 3.14f) / NumberOfPoints;

    float radiusPixels = radius;
    
    float angle = 0.0f;

    glm::vec2 startPos = glm::vec2(radius, 0) + pos;
    glm::vec2 oldPos = startPos;
    
    for(int i = 0; i < NumberOfPoints; ++i)
    {
	glm::vec2 newPos = glm::vec2(cos(angle) * radius, sin(angle) * radius) + pos;

	add(oldPos, newPos, color);

	oldPos = newPos;
	angle += DeltaAngle;
    }

    add(oldPos, startPos, color);
}

void LineRenderer::drawAABB(const AABB& aabb, const glm::vec4& color)
{
    glm::vec2 a = aabb.lower;
    glm::vec2 b = aabb.lower + glm::vec2(0, aabb.upper.y - aabb.lower.y);
    glm::vec2 c = aabb.upper;
    glm::vec2 d = aabb.lower + glm::vec2(aabb.upper.x - aabb.lower.x, 0);

    add(a, b, color);
    add(b, c, color);
    add(c, d, color);
    add(d, a, color);
}

void LineRenderer::drawPolygon(const PolygonShape& shape, const glm::vec4& color, const glm::vec2& position)
{
    for(int i = 0; i < shape.count; ++i)
    {
	glm::vec2 a = shape.vertices[i];
	glm::vec2 b = i + 1 < shape.count ? shape.vertices[i+1] : shape.vertices[0];

	add(a + position, b + position, color);
    }
}

void LineRenderer::drawPolygon(const PolygonShape& shape, const glm::vec4& color, const Transform& transform)
{
    for(int i = 0; i < shape.count; ++i)
    {
	glm::vec2 a = shape.vertices[i];
	glm::vec2 b = i + 1 < shape.count ? shape.vertices[i+1] : shape.vertices[0];

	a = transform.mul(a);
	b = transform.mul(b);
	
	add(a, b, color);
    }
}
