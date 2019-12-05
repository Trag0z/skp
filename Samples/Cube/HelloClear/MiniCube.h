#pragma once
#include <assert.h>
#include <gxm.h>
// #include <kernel.h>

enum Color {
    WHITE = 0xffffff00,
    BLACK = 0x00000000,
    GREEN = 0x00ff0000,
    ORANGE = 0xff990000,
    BLUE = 0x0000ff00,
    RED = 0xff000000,
    YELLOW = 0xffff0000
};

struct Vertex {
    float position[3];
    uint32_t color;
};

// NOTE: When drawing these, rotate the vertices by m_rotation and translate
// them by m_position. Then draw textures on two triangles per texture, by
// indeces
class MiniCube {
    float m_position[3];
    float m_rotation[3];
    Vertex m_vertices[6][4];
    SceGxmTexture *m_texture[6];

    static Vertex *s_vertices;
    static int32_t s_verticesUId;
    static const int s_numVertices = 27 * 6 * 4;

    static uint16_t *s_indeces;
    static int32_t s_indicesUId;

  public:
    enum Side { FRONT = 0, BACK = 1, LEFT = 2, RIGHT = 3, TOP = 4, BOTTOM = 5 };

    static void init() {
        s_vertices = (Vertex *)graphicsAlloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
            s_numVertices * sizeof(Vertex), 3, SCE_GXM_MEMORY_ATTRIB_READ,
            &s_verticesUId);

        // What is allingment here? WHy is it 2 in main.cpp?
        s_indeces = (uint16_t *)graphicsAlloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
            6 * 6 * sizeof(uint16_t), 2, SCE_GXM_MEMORY_ATTRIB_READ,
            &s_indicesUId);

        int count = 0;
        for (int side = 0; side < 6; ++side) {
            int baseIndex = side * 4;

            s_indeces[count++] = baseIndex;
            s_indeces[count++] = baseIndex + 1;
            s_indeces[count++] = baseIndex + 2;

            s_indeces[count++] = baseIndex + 2;
            s_indeces[count++] = baseIndex + 3;
            s_indeces[count++] = baseIndex;
        }
    }

    // NOTE: Does this need MiniCube diameter?
    static void create(MiniCube &mc, const float position[3],
                       const int cubeLocation[3]) {
        mc.m_position[0] = position[0];
        mc.m_position[1] = position[1];
        mc.m_position[2] = position[2];

        Vertex *side = mc.m_vertices[FRONT];
        side[0] = {
            {-0.5f, -0.5f, -0.5f}
        };
        side[1] = {{0.5f, -0.5f, -0.5f}};
        side[2] = {{0.5f, 0.5f, -0.5f}};
        side[3] = {{-0.5f, 0.5f, -0.5f}};
        side = mc.m_vertices[BACK];
        side[0] = {{0.5f, -0.5f, 0.5f}};
        side[1] = {{-0.5f, -0.5f, 0.5f}};
        side[2] = {{0.5f, 0.5f, 0.5f}};
        side[3] = {{-0.5f, 0.5f, 0.5f}};
        side = mc.m_vertices[LEFT];
        side[0] = {{-0.5f, -0.5f, 0.5f}};
        side[1] = {{-0.5f, -0.5f, -0.5f}};
        side[2] = {{-0.5f, 0.5f, -0.5f}};
        side[3] = {{-0.5f, 0.5f, 0.5f}};
        side = mc.m_vertices[RIGHT];
        side[0] = {{0.5f, -0.5f, -0.5f}};
        side[1] = {{0.5f, -0.5f, 0.5f}};
        side[2] = {{0.5f, 0.5f, 0.5f}};
        side[3] = {{0.5f, 0.5f, -0.5f}};
        side = mc.m_vertices[TOP];
        side[0] = {{-0.5f, 0.5f, 0.5f}};
        side[1] = {{0.5f, 0.5f, 0.5f}};
        side[2] = {{0.5f, 0.5f, -0.5f}};
        side[3] = {{-0.5f, 0.5f, -0.5f}};
        side = mc.m_vertices[BOTTOM];
        side[0] = {{-0.5f, -0.5f, -0.5f}};
        side[1] = {{0.5f, -0.5f, -0.5f}};
        side[2] = {{0.5f, -0.5f, 0.5f}};
        side[3] = {{-0.5f, -0.5f, 0.5f}};

        auto setColors = [&mc](Color front, Color back, Color left, Color right,
                               Color top, Color bottom) {
            Color colors[6] = {front, back, left, right, top, bottom};
            for (int side = 0; side < 6; ++side) {
                for (auto v : mc.m_vertices[side]) {
                    v.color = static_cast<uint32_t>(colors[side]);
                }
            }
        };

        if (cubeLocation[0] == 0) {     // X Left
            if (cubeLocation[1] == 0) { // Y Top
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, GREEN, BLACK, ORANGE, BLACK);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, GREEN, BLACK, ORANGE, BLACK);
                } else {
                    setColors(BLACK, YELLOW, GREEN, BLACK, ORANGE, BLACK);
                }
            } else if (cubeLocation[1] == 1) { // Y Middle
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, GREEN, BLACK, BLACK, BLACK);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, GREEN, BLACK, BLACK, BLACK);
                } else {
                    setColors(BLACK, YELLOW, GREEN, BLACK, BLACK, BLACK);
                }
            } else { // Y Bottom
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, GREEN, BLACK, BLACK, RED);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, GREEN, BLACK, BLACK, RED);
                } else {
                    setColors(BLACK, YELLOW, GREEN, BLACK, BLACK, RED);
                }
            }
        } else if (cubeLocation[0] == 1) { // X Middle
            if (cubeLocation[1] == 0) {    // Y Top
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, BLACK, BLACK, ORANGE, BLACK);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, BLACK, BLACK, ORANGE, BLACK);
                } else {
                    setColors(BLACK, BLACK, BLACK, YELLOW, ORANGE, BLACK);
                }
            } else if (cubeLocation[1] == 1) { // Y Middle
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, BLACK, BLACK, BLACK, BLACK);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, BLACK, BLACK, BLACK, BLACK);
                } else {
                    setColors(BLACK, YELLOW, BLACK, BLACK, BLACK, BLACK);
                }
            } else { // Y Bottom
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, BLACK, BLACK, BLACK, RED);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, BLACK, BLACK, BLACK, RED);
                } else {
                    setColors(BLACK, YELLOW, BLACK, BLACK, BLACK, RED);
                }
            }
        } else {                        // X Right
            if (cubeLocation[1] == 0) { // Y Top
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, BLACK, BLUE, ORANGE, BLACK);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, BLACK, BLUE, ORANGE, BLACK);
                } else {
                    setColors(BLACK, YELLOW, BLACK, BLUE, ORANGE, BLACK);
                }
            } else if (cubeLocation[1] == 1) { // Y Middle
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, BLACK, BLUE, BLACK, BLACK);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, BLACK, BLUE, BLACK, BLACK);
                } else {
                    setColors(BLACK, YELLOW, BLACK, BLUE, BLACK, BLACK);
                }
            } else { // Y Bottom
                if (cubeLocation[2] == 0) {
                    setColors(WHITE, BLACK, BLACK, RED, BLACK, RED);
                } else if (cubeLocation[2] == 1) {
                    setColors(BLACK, BLACK, BLACK, RED, BLACK, RED);
                } else {
                    setColors(BLACK, YELLOW, BLACK, RED, BLACK, RED);
                }
            }
        }
    }

    void setPosition(float pos[3]) {
        m_position[0] = pos[0];
        m_position[1] = pos[1];
        m_position[2] = pos[2];
    }

    // NOTE: It this degrees or angle?
    void rotate(float degrees, int axis) {
        assert(axis < 3);
        m_rotation[axis] += degrees;
    }
};