#pragma once
#include "Helpers.h"
#include <cstring>
#include <gxm.h>
#include <kernel.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;

extern Matrix4 g_finalTransformation;
extern Matrix4 g_finalRotation;
extern const SceGxmProgramParameter *g_wvpParam;
extern const SceGxmProgramParameter *g_rotParam;
extern const SceGxmProgramParameter *g_localToWorldParam;

enum Color {
    WHITE = 0xffffffff,
    BLACK = 0xff000000,
    GREEN = 0xff00ff00,
    ORANGE = 0xff0099ff,
    BLUE = 0xffff0000,
    RED = 0xff0000ff,
    YELLOW = 0xff00ffff
};

/*	Data structure for basic geometry */
typedef struct Vertex {
    float position[3]; // Easier to index.
    float normal[3];   // Contains normal now.
    uint32_t color;    // Data gets expanded to float 4 in vertex shader.
} Vertex;

const static Vertex s_defaultVertices[24] = {
    // Front
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, 0},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, 0},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, 0},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, 0},
    // Back
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, 0},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, 0},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, 0},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, 0},
    // Left
    {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, 0},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, 0},
    {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, 0},
    {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, 0},
    // Right
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, 0},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, 0},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, 0},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, 0},
    // Top
    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, 0},
    {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, 0},
    {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, 0},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, 0},
    // Bottom
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, 0},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, 0},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, 0},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, 0},
};

struct MiniCube {
    Vertex *vertices;
    Vector3 position;
    Vector3 rotation;

    int32_t verticesUId;
};

inline static void getLocalToWorldTransform(const MiniCube &mc, Matrix4 &out) {
    out =
        Matrix4::translation(mc.position); // * Matrix4::rotation(mc.rotation);
}

inline void renderMiniCube(const MiniCube &mc, SceGxmContext *context, const uint16_t *indices) {
    void *vertexDefaultBuffer;
    sceGxmReserveVertexDefaultUniformBuffer(context, &vertexDefaultBuffer);
    sceGxmSetUniformDataF(vertexDefaultBuffer, g_wvpParam, 0, 16,
                          (float *)&g_finalTransformation);
    sceGxmSetUniformDataF(vertexDefaultBuffer, g_rotParam, 0, 16,
                          (float *)&g_finalRotation);

    Matrix4 localToWorld;
    getLocalToWorldTransform(mc, localToWorld);
    sceGxmSetUniformDataF(vertexDefaultBuffer, g_localToWorldParam, 0, 16,
                          (float *)&localToWorld);

    /* draw the spinning triangle */
    sceGxmSetVertexStream(context, 0, mc.vertices);
    sceGxmDraw(context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16,
               indices, 6 * 6);
}

static void setColors(MiniCube &mc, Color front, Color back, Color left,
                      Color right, Color top, Color bottom) {
    Color colors[6] = {front, back, left, right, top, bottom};
    for (int side = 0; side < 6; ++side) {
        for (int vertex = 0; vertex < 4; ++vertex) {
            mc.vertices[side * 4 + vertex].color = colors[side];
        }
    }
}

inline MiniCube createMiniCube(Vector3 pos, int cubeLocation[3]) {
    MiniCube mc;
    mc.position = pos;
    mc.rotation = Vector3(0.0f, 0.0f, 0.0f);

    mc.vertices = (Vertex *)graphicsAlloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 4 * 6 * sizeof(Vertex), 4,
        SCE_GXM_MEMORY_ATTRIB_READ, &mc.verticesUId);

    memcpy(mc.vertices, s_defaultVertices, sizeof(Vertex) * 24);

    if (cubeLocation[0] == 0) {     // X Left
        if (cubeLocation[1] == 0) { // Y Top
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, GREEN, BLACK, ORANGE, BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, GREEN, BLACK, ORANGE, BLACK);
            } else {
                setColors(mc, BLACK, YELLOW, GREEN, BLACK, ORANGE, BLACK);
            }
        } else if (cubeLocation[1] == 1) { // Y Middle
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, GREEN, BLACK, BLACK, BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, GREEN, BLACK, BLACK, BLACK);
            } else {
                setColors(mc, BLACK, YELLOW, GREEN, BLACK, BLACK, BLACK);
            }
        } else { // Y Bottom
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, GREEN, BLACK, BLACK, RED);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, GREEN, BLACK, BLACK, RED);
            } else {
                setColors(mc, BLACK, YELLOW, GREEN, BLACK, BLACK, RED);
            }
        }
    } else if (cubeLocation[0] == 1) { // X Middle
        if (cubeLocation[1] == 0) {    // Y Top
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, BLACK, BLACK, ORANGE, BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, BLACK, BLACK, ORANGE, BLACK);
            } else {
                setColors(mc, BLACK, YELLOW, BLACK, BLACK, ORANGE, BLACK);
            }
        } else if (cubeLocation[1] == 1) { // Y Middle
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK);
            } else {
                setColors(mc, BLACK, YELLOW, BLACK, BLACK, BLACK, BLACK);
            }
        } else { // Y Bottom
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, BLACK, BLACK, BLACK, RED);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, BLACK, BLACK, BLACK, RED);
            } else {
                setColors(mc, BLACK, YELLOW, BLACK, BLACK, BLACK, RED);
            }
        }
    } else {                        // X Right
        if (cubeLocation[1] == 0) { // Y Top
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, BLACK, BLUE, ORANGE, BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, BLACK, BLUE, ORANGE, BLACK);
            } else {
                setColors(mc, BLACK, YELLOW, BLACK, BLUE, ORANGE, BLACK);
            }
        } else if (cubeLocation[1] == 1) { // Y Middle
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, BLACK, BLUE, BLACK, BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, BLACK, BLUE, BLACK, BLACK);
            } else {
                setColors(mc, BLACK, YELLOW, BLACK, BLUE, BLACK, BLACK);
            }
        } else { // Y Bottom
            if (cubeLocation[2] == 0) {
                setColors(mc, WHITE, BLACK, BLACK, BLUE, BLACK, RED);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, BLACK, BLACK, BLACK, BLUE, BLACK, RED);
            } else {
                setColors(mc, BLACK, YELLOW, BLACK, BLUE, BLACK, RED);
            }
        }
    }

    return mc;
}
