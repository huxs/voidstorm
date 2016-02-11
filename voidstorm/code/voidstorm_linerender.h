#pragma once

struct LineVertex
{
    glm::vec3 pos;
    glm::vec4 color;
};

class LineRenderer
{
public:
    LineRenderer(dcfx::Context* renderCtx);
    ~LineRenderer();
    	
    void add(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color);
    void dispatch(int view);
    void reset();

    void drawCircle(const glm::vec2& pos, float radius, const glm::vec4& color);
    void drawAABB(const AABB& aabb, const glm::vec4& color);
    void drawPolygon(const PolygonShape& shape, const glm::vec4& color, const glm::vec2& position);
    void drawPolygon(const PolygonShape& shape, const glm::vec4& color, const Transform& transform);
    
private:
    dcfx::Context* renderCtx;
    dcfx::VertexDecl vertexDecl;
    dcfx::BufferHandle vertexBuffer;
    dcfx::ProgramHandle program;
    LineVertex* vertices;
    size_t vertexCount;
};



