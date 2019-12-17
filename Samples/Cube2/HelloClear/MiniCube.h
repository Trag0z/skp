#pragma once
#include "Helpers.h"
#include <cstring>
#include <gxm.h>
#include <kernel.h>
#include <libdbg.h>
#include <rtc.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;

extern Matrix4 g_finalTransformation;
extern Matrix4 g_finalRotation;
extern const SceGxmProgramParameter *g_wvpParam;
extern const SceGxmProgramParameter *g_rotParam;
extern const SceGxmProgramParameter *g_localToWorldParam;

extern bool g_animationState;
extern const float g_miniCubeHalfSize;

static SceRtcTick s_lastTick;
const static float c_animationSpeed = 2e-4;
static float s_interpolationValue;

enum Color {
    WHITE = 0xffffffff,
    BLACK = 0xff000000,
    GREEN = 0xff00ff00,
    ORANGE = 0xff0099ff,
    BLUE = 0xffff0000,
    RED = 0xff0000ff,
    YELLOW = 0xff00ffff
};

struct Vertex {
    float position[3];
    float normal[3];
    uint32_t color;
};

struct MiniCube {
    Vertex *vertices;
    Vector3 position;
    Quat rotation;

    Vector3 startPosition;
    Quat startRotation;

    Vector3 targetPosition;
    Quat targetRotation;

    int32_t verticesUId;
};

const static Vertex s_defaultVertices[24] = {
    // Front
    {{-g_miniCubeHalfSize, -g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, 0.0f, -1.0f},
     0},
    {{g_miniCubeHalfSize, -g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, 0.0f, -1.0f},
     0},
    {{g_miniCubeHalfSize, g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, 0.0f, -1.0f},
     0},
    {{-g_miniCubeHalfSize, g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, 0.0f, -1.0f},
     0},
    // Back
    {{g_miniCubeHalfSize, -g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, 0.0f, 1.0f},
     0},
    {{-g_miniCubeHalfSize, -g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, 0.0f, 1.0f},
     0},
    {{-g_miniCubeHalfSize, g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, 0.0f, 1.0f},
     0},
    {{g_miniCubeHalfSize, g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, 0.0f, 1.0f},
     0},
    // Left
    {{-g_miniCubeHalfSize, -g_miniCubeHalfSize, g_miniCubeHalfSize},
     {-1.0f, 0.0f, 0.0f},
     0},
    {{-g_miniCubeHalfSize, -g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {-1.0f, 0.0f, 0.0f},
     0},
    {{-g_miniCubeHalfSize, g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {-1.0f, 0.0f, 0.0f},
     0},
    {{-g_miniCubeHalfSize, g_miniCubeHalfSize, g_miniCubeHalfSize},
     {-1.0f, 0.0f, 0.0f},
     0},
    // Right
    {{g_miniCubeHalfSize, -g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {1.0f, 0.0f, 0.0f},
     0},
    {{g_miniCubeHalfSize, -g_miniCubeHalfSize, g_miniCubeHalfSize},
     {1.0f, 0.0f, 0.0f},
     0},
    {{g_miniCubeHalfSize, g_miniCubeHalfSize, g_miniCubeHalfSize},
     {1.0f, 0.0f, 0.0f},
     0},
    {{g_miniCubeHalfSize, g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {1.0f, 0.0f, 0.0f},
     0},
    // Top
    {{-g_miniCubeHalfSize, -g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, -1.0f, 0.0f},
     0},
    {{g_miniCubeHalfSize, -g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, -1.0f, 0.0f},
     0},
    {{g_miniCubeHalfSize, -g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, -1.0f, 0.0f},
     0},
    {{-g_miniCubeHalfSize, -g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, -1.0f, 0.0f},
     0},
    // Bottom
    {{-g_miniCubeHalfSize, g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, 1.0f, 0.0f},
     0},
    {{g_miniCubeHalfSize, g_miniCubeHalfSize, -g_miniCubeHalfSize},
     {0.0f, 1.0f, 0.0f},
     0},
    {{g_miniCubeHalfSize, g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, 1.0f, 0.0f},
     0},
    {{-g_miniCubeHalfSize, g_miniCubeHalfSize, g_miniCubeHalfSize},
     {0.0f, 1.0f, 0.0f},
     0},
};

inline static void getLocalToWorldTransform(const MiniCube &mc, Matrix4 &out) {
    out = Matrix4::translation(mc.position) * Matrix4::rotation(mc.rotation);
}

inline void renderMiniCube(const MiniCube &mc, SceGxmContext *context,
                           const uint16_t *indices) {
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
    mc.position = mc.startPosition = pos;
    mc.rotation = mc.startRotation = Quat::identity();

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

inline void setAnimationInterpolationValue(float t) {
    s_interpolationValue = t;
}

inline void progressAnimations(MiniCube *miniCubes) {
    if (g_animationState == ANIMSTATE_TOUCH) {
        for (int i = 0; i < 27; ++i) {
            MiniCube &mc = miniCubes[i];
            if (mc.position != mc.targetPosition) {
                mc.position = Vector3::lerp(
                    s_interpolationValue, mc.startPosition, mc.targetPosition);
            }
        }
        return;
    }

    if (g_animationState != ANIMSTATE_ANIMATING)
        return;

    SceRtcTick currentTick;
    sceRtcGetCurrentTick(&currentTick);
    float interpolationStep =
        static_cast<float>(currentTick - s_lastTick) * c_animationSpeed;
    if (s_interpolationValue < 0.5f) {
        s_interpolationValue =
            std::max(s_interpolationValue - interpolationStep, 0.0f);
    } else {
        s_interpolationValue =
            std::min(s_interpolationValue + interpolationStep, 1.0f);
    }
    s_lastTick = currentTick;

    bool animationsFinished = true;
    for (int i = 0; i < 27; ++i) {
        MiniCube &mc = miniCubes[i];
        if (mc.position != mc.targetPosition) {
            if (s_interpolationValue == 0.0f) {
                mc.position = mc.targetPosition = mc.startPosition;
            } else if (s_interpolationValue == 1.0f) {
                mc.position = mc.startPosition = mc.targetPosition;
            } else {
                mc.position = Vector3::lerp(
                    s_interpolationValue, mc.startPosition, mc.targetPosition);
                animationsFinished = false;
            }
        }
    }

    if (animationsFinished)
        g_animationState = ANIMSTATE_NO_ANIM;
}

enum Dimension { DIM_X = 0, DIM_Y = 1, DIM_Z = 2 };

static inline MiniCube *getMiniCubeByLocation(MiniCube *miniCubes, int x, int y,
                                              int z) {
    return &miniCubes[x + y * 3 + z * 9];
}

static inline float toRad(float degrees) {
    return degrees * 3.14159265359 / 180;
}

static inline void setTargetRotation(MiniCube &mc, float degrees,
                                     Dimension dim) {
    if (dim == DIM_X) {
        mc.targetRotation = mc.startRotation * Quat::rotationX(toRad(degrees));
    } else if (dim = DIM_Y) {
        mc.targetRotation = mc.startRotation * Quat::rotationY(toRad(degrees));
    } else {
        mc.targetRotation = mc.startRotation * Quat::rotationY(toRad(degrees));
    }
    normalize(mc.targetRotation);
}

void setAnimation(MiniCube *miniCubes, int layer, Dimension dimension,
                  bool clockwise) {
    SCE_DBG_ALWAYS_ASSERT(layer < 3);

    MiniCube *selectedLayer[3][3];
    switch (dimension) {
    case DIM_X:
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                selectedLayer[i][j] =
                    getMiniCubeByLocation(miniCubes, layer, i, j);
            }
        }
        if (clockwise) {
            selectedLayer[0][0]->targetPosition =
                selectedLayer[0][2]->startPosition;
            selectedLayer[0][1]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[0][2]->targetPosition =
                selectedLayer[2][2]->startPosition;
            selectedLayer[1][0]->targetPosition =
                selectedLayer[0][1]->startPosition;
            selectedLayer[1][1]->targetPosition =
                selectedLayer[1][1]->startPosition;
            selectedLayer[1][2]->targetPosition =
                selectedLayer[2][1]->startPosition;
            selectedLayer[2][0]->targetPosition =
                selectedLayer[0][0]->startPosition;
            selectedLayer[2][1]->targetPosition =
                selectedLayer[1][0]->startPosition;
            selectedLayer[2][2]->targetPosition =
                selectedLayer[2][0]->startPosition;
        } else {
            selectedLayer[0][0]->targetPosition =
                selectedLayer[2][0]->startPosition;
            selectedLayer[0][1]->targetPosition =
                selectedLayer[1][0]->startPosition;
            selectedLayer[0][2]->targetPosition =
                selectedLayer[0][0]->startPosition;
            selectedLayer[1][0]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[1][1]->targetPosition =
                selectedLayer[1][1]->startPosition;
            selectedLayer[1][2]->targetPosition =
                selectedLayer[0][1]->startPosition;
            selectedLayer[2][0]->targetPosition =
                selectedLayer[2][2]->startPosition;
            selectedLayer[2][1]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[2][2]->targetPosition =
                selectedLayer[0][2]->startPosition;
        }
        break;
    case DIM_Y:
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                selectedLayer[i][j] =
                    getMiniCubeByLocation(miniCubes, i, layer, j);
            }
        }
        if (!clockwise) {
            selectedLayer[0][0]->targetPosition =
                selectedLayer[0][2]->startPosition;
            selectedLayer[0][1]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[0][2]->targetPosition =
                selectedLayer[2][2]->startPosition;
            selectedLayer[1][0]->targetPosition =
                selectedLayer[0][1]->startPosition;
            selectedLayer[1][1]->targetPosition =
                selectedLayer[1][1]->startPosition;
            selectedLayer[1][2]->targetPosition =
                selectedLayer[2][1]->startPosition;
            selectedLayer[2][0]->targetPosition =
                selectedLayer[0][0]->startPosition;
            selectedLayer[2][1]->targetPosition =
                selectedLayer[1][0]->startPosition;
            selectedLayer[2][2]->targetPosition =
                selectedLayer[2][0]->startPosition;
        } else {
            selectedLayer[0][0]->targetPosition =
                selectedLayer[2][0]->startPosition;
            selectedLayer[0][1]->targetPosition =
                selectedLayer[1][0]->startPosition;
            selectedLayer[0][2]->targetPosition =
                selectedLayer[0][0]->startPosition;
            selectedLayer[1][0]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[1][1]->targetPosition =
                selectedLayer[1][1]->startPosition;
            selectedLayer[1][2]->targetPosition =
                selectedLayer[0][1]->startPosition;
            selectedLayer[2][0]->targetPosition =
                selectedLayer[2][2]->startPosition;
            selectedLayer[2][1]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[2][2]->targetPosition =
                selectedLayer[0][2]->startPosition;
        }
        break;
    case DIM_Z:
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                selectedLayer[i][j] =
                    getMiniCubeByLocation(miniCubes, i, j, layer);
            }
        }
        if (clockwise) {
            selectedLayer[0][0]->targetPosition =
                selectedLayer[0][2]->startPosition;
            selectedLayer[0][1]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[0][2]->targetPosition =
                selectedLayer[2][2]->startPosition;
            selectedLayer[1][0]->targetPosition =
                selectedLayer[0][1]->startPosition;
            selectedLayer[1][1]->targetPosition =
                selectedLayer[1][1]->startPosition;
            selectedLayer[1][2]->targetPosition =
                selectedLayer[2][1]->startPosition;
            selectedLayer[2][0]->targetPosition =
                selectedLayer[0][0]->startPosition;
            selectedLayer[2][1]->targetPosition =
                selectedLayer[1][0]->startPosition;
            selectedLayer[2][2]->targetPosition =
                selectedLayer[2][0]->startPosition;
        } else {
            selectedLayer[0][0]->targetPosition =
                selectedLayer[2][0]->startPosition;
            selectedLayer[0][1]->targetPosition =
                selectedLayer[1][0]->startPosition;
            selectedLayer[0][2]->targetPosition =
                selectedLayer[0][0]->startPosition;
            selectedLayer[1][0]->targetPosition =
                selectedLayer[2][1]->startPosition;
            selectedLayer[1][1]->targetPosition =
                selectedLayer[1][1]->startPosition;
            selectedLayer[1][2]->targetPosition =
                selectedLayer[0][1]->startPosition;
            selectedLayer[2][0]->targetPosition =
                selectedLayer[2][2]->startPosition;
            selectedLayer[2][1]->targetPosition =
                selectedLayer[1][2]->startPosition;
            selectedLayer[2][2]->targetPosition =
                selectedLayer[0][2]->startPosition;
        }
        break;
    default:
        break;
    }

    if (clockwise) {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                setTargetRotation(*selectedLayer[row][col], 90.0f, dimension);
            }
        }
    } else {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                setTargetRotation(*selectedLayer[row][col], -90.0f, dimension);
            }
        }
    }

    sceRtxGetCurrentTick(s_lastTick);
}