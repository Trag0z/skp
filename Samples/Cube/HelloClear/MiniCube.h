#include <assert.h>
#include <gxm.h>
// #include <kernel.h>

struct Vertex {
    float position[3];
    // float normal[3]; // If uncommenting this, change MiniCube::create()
    // accordingly
};

// NOTE: When drawing these, rotate the vertices by m_rotation and translate
// them by m_position. Then draw textures on two triangles per texture, by
// indeces
class MiniCube {
    float m_position[3];
    float m_rotation[3];
    Vertex m_vertices[6 * 4];
    SceGxmTexture *m_texture[6];

    static Vertex *s_vertices;
    static int32_t s_verticesUId;
    static const size_t s_numVertices = 27 * 6 * 4;

    static uint16_t *s_indeces;
    static int32_t s_indecesUId;

  public:
    static void init() {
        s_vertices = (Vertex *)graphicsAlloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
            s_numVertices * sizeof(Vertex), )

            s_indeces = (uint16_t *)graphicsAlloc(
                SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
                6 * 6 * sizeof(uint16_t), 2, SCE_GXM_MEMORY_ATTRIB_READ,
                &s_basicIndiceUId);

        size_t count = 0;
        for (size_t side = 0; side < 6; ++side) {
            size_t baseIndex = side * 4;

            s_indeces[count++] = baseIndex;
            // Check order of vertices in lecture program and set here
            // accordingly. Do I need more vertices because of anti-clockwise
            // alignment?
        }
    }

    static void create(MiniCube &mc, const float position[3]) {
        mc.m_position[0] = position[0];
        mc.m_position[1] = position[1];
        mc.m_position[2] = position[2];
        // Front side, clockwise starting in top left
        mc.m_vertices[0] = {-0.5f, -0.5f, -0.5f};
        mc.m_vertices[1] = {0.5f, -0.5f, -0.5f};
        mc.m_vertices[2] = {0.5f, 0.5f, -0.5f};
        mc.m_vertices[3] = {-0.5f, 0.5f, -0.5f};
        // Back side, same thing
        mc.m_vertices[4] = {-0.5f, 0.5f, 0.5f};
        mc.m_vertices[5] = {-0.5f, 0.5f, 0.5f};
        mc.m_vertices[6] = {-0.5f, 0.5f, 0.5f};
        mc.m_vertices[7] = {-0.5f, 0.5f, 0.5f};
    }

    void setPosition(float pos[3]) {
        m_position[0] = pos[0];
        m_position[1] = pos[1];
        m_position[2] = pos[2];
    }

    // NOTE: It this degrees or angle?
    void rotate(float degrees, size_t axis) {
        assert(axis < 3);
        m_rotation[axis] += degrees;
    }
};