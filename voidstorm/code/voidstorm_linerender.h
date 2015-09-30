#pragma once

struct LineVertex
{
    glm::vec3 m_pos;
};

class LineRenderer
{
public:
    LineRenderer(dcfx::Context* renderCtx);
    ~LineRenderer();
    	
    void add(const glm::vec2& from, const glm::vec2& to);
    void dispatch(int view);
    void reset();

    void drawCircle(const glm::vec2& pos, float radius);
    void drawAABB(const AABB& aabb);
    void drawPolygon(const PolygonShape& shape);
    
private:

    // TODO: Debug malloc.
    static const int BatchSize = 75000;
    
    dcfx::Context* renderCtx;
    dcfx::VertexDecl vertexDecl;
    dcfx::BufferHandle vertexBuffer;
    dcfx::ProgramHandle program;
    LineVertex* vertices;
    size_t vertexCount;
};



