#pragma once
#include "Helpers.h"
#include <assert.h>
#include <gxm.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;
// #include <kernel.h>

// NOTE: Maybe these should all start with ff?
enum TextureColors {
    WHITE = 0,
    BLACK = 1,
    GREEN = 2,
    ORANGE = 3,
    BLUE = 4,
    RED = 5,
    YELLOW = 6
};

struct Vertex {
    float position[3];
    // uint32_t color;

    Vertex(float posX, float posY, float posZ) {
        position[0] = posX;
        position[1] = posY;
        position[2] = posZ;
    }
};

class MiniCube {
    Vector3 m_position;
    // float m_rotation[3];
    Quat m_orientation;
    // Vertex m_vertices[6][4];
    SceGxmTexture *m_textures[6];

    static Vertex *s_vertices;
    static int32_t s_verticesUId;
    static const int s_numVertices = 27 * 6 * 4;

    static uint16_t *s_indices;
    static int32_t s_indicesUId;

    static SceGxmTexture s_textures[7];

  public:
    enum Side { FRONT = 0, BACK = 1, LEFT = 2, RIGHT = 3, TOP = 4, BOTTOM = 5 };

    static void init();

    void rotate(float radians, int axis);

    // NOTE: Does this need MiniCube diameter?
    static void create(MiniCube &mc, const float position[3],
                       const int cubeLocation[3]);

    // void setPosition(float pos[3]) {
    //     m_position[0] = pos[0];
    //     m_position[1] = pos[1];
    //     m_position[2] = pos[2];
    // }

    const Vector3 *position() const { return &m_position; }

    // const Vertex *vertices() const { return &m_vertices; }

    SceGxmTexture **textures() { return m_textures; }

    static const Vertex *vertices() { return s_vertices; }

    static const uint16_t *indeces() { return s_indices; }

    void localToWorldTransform(Matrix4 &out) const {
        out =
            Matrix4::translation(m_position) * Matrix4::rotation(m_orientation);
    }
};