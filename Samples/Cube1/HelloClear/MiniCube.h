#pragma once
#include "Globals.h"
#include "Helpers.h"
#include <algorithm>
#include <cstring>
#include <gxm.h>
#include <kernel.h>
#include <libdbg.h>
#include <limits>
#include <rtc.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;

#define PI 3.14159265359f

extern Matrix4 g_finalTransformation;
extern Matrix4 g_finalRotation;
extern const SceGxmProgramParameter* g_wvpParam;
// extern const SceGxmProgramParameter* g_rotParam;
extern const SceGxmProgramParameter* g_localToWorldParam;
extern const float g_miniCubeHalfSize;
extern bool g_animationStarted;

// NOTE: Why does this have to bere up here?
enum Dimension { DIM_X = 0, DIM_Y = 1, DIM_Z = 2 };

struct MiniCube;
enum AnimationState;

// TODO: make the declaration consistent across files
extern MiniCube* g_miniCubes[3][3][3];

extern AnimationState g_animationState;

static SceRtcTick s_lastTick;
const static float c_animationSpeed = 2e-7;
static float s_interpolationValue;
static float s_targetInterpolation = std::numeric_limits<float>::max();
static Dimension s_rotatingDimension;
static MiniCube* s_rotatingMiniCubes[3][3];
static MiniCube** s_correspondingGlobalPointers[3][3];

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
    Vertex* vertices;
    Vector3 position;
    Quat orientation;

    float startRotation, targetRotation;

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

inline static void getLocalToWorldTransform(const MiniCube& mc, Matrix4& out) {
    // translate first, then rotate
    out = Matrix4::rotation(normalize(mc.orientation * Quat::identity())) *
          Matrix4::translation(mc.position);
}

inline void renderMiniCubes(SceGxmContext* context, const uint16_t* indices) {
    void* vertexDefaultBuffer;
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            for (int z = 0; z < 3; ++z) {
                sceGxmReserveVertexDefaultUniformBuffer(context,
                                                        &vertexDefaultBuffer);
                sceGxmSetUniformDataF(vertexDefaultBuffer, g_wvpParam, 0, 16,
                                      (float*)&g_finalTransformation);
                // sceGxmSetUniformDataF(vertexDefaultBuffer, g_rotParam, 0, 16,
                //                       (float*)&g_finalRotation);

                Matrix4 localToWorld;
                getLocalToWorldTransform(*g_miniCubes[x][y][z], localToWorld);
                sceGxmSetUniformDataF(vertexDefaultBuffer, g_localToWorldParam,
                                      0, 16, (float*)&localToWorld);

                sceGxmSetVertexStream(context, 0,
                                      g_miniCubes[x][y][z]->vertices);
                sceGxmDraw(context, SCE_GXM_PRIMITIVE_TRIANGLES,
                           SCE_GXM_INDEX_FORMAT_U16, indices, 6 * 6);
            }
        }
    }
}

static void setColors(MiniCube& mc, Color front, Color back, Color left,
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
    mc.orientation = Quat::identity();
    mc.startRotation = mc.targetRotation = 0.0f;

    mc.vertices = (Vertex*)graphicsAlloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 4 * 6 * sizeof(Vertex), 4,
        SCE_GXM_MEMORY_ATTRIB_READ, &mc.verticesUId);

    // TODO: Vertices can store position, so position attribute is obsolete
    std::memcpy(mc.vertices, s_defaultVertices, sizeof(Vertex) * 24);

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
    if (t < 0.0f) {
        t += 1.0f;
    }
    s_interpolationValue = t;
}

inline Quat lerpOrientation(float t, float a, float b) {
    Quat result;
    if (s_rotatingDimension == DIM_X) {
        result = normalize(Quat::rotationX(a + (t * (b - a))));
    } else if (s_rotatingDimension == DIM_Y) {
        result = normalize(Quat::rotationY(a + (t * (b - a))));
    } else {
        result = normalize(Quat::rotationZ(a + (t * (b - a))));
    }
    return result;
}

inline void progressAnimations() {
    SceRtcTick currentTick;
    sceRtcGetCurrentTick(&currentTick);
    if (g_animationState == ANIMSTATE_TOUCHING && g_animationStarted) {
        s_lastTick = currentTick;
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                MiniCube& mc = *s_rotatingMiniCubes[x][y];
                mc.orientation = lerpOrientation(
                    s_interpolationValue, mc.startRotation, mc.targetRotation);
            }
        }
        return;
    }

    if (g_animationState != ANIMSTATE_ANIMATING) {
        return;
    }

    if (s_targetInterpolation == std::numeric_limits<float>::max()) {
        s_targetInterpolation = round(s_interpolationValue * 4.0f) / 4.0f;
    }

    float interpolationStep =
        static_cast<float>(currentTick.tick - s_lastTick.tick) *
        c_animationSpeed;

    if (s_targetInterpolation < s_interpolationValue) {
        s_interpolationValue = std::max(
            s_interpolationValue - interpolationStep, s_targetInterpolation);
    } else {
        s_interpolationValue = std::min(
            s_interpolationValue + interpolationStep, s_targetInterpolation);
    }

    // Progress the animations
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            MiniCube& mc = *s_rotatingMiniCubes[x][y];
            mc.orientation = lerpOrientation(
                s_interpolationValue, mc.startRotation, mc.targetRotation);
        }
    }

    if (s_interpolationValue != s_targetInterpolation) {
        s_lastTick = currentTick;
        return;
    }

    // Animations have finished
    g_animationState = ANIMSTATE_NO_ANIM;

    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            MiniCube& mc = *s_rotatingMiniCubes[x][y];
            mc.startRotation += s_targetInterpolation * 2.0f * PI;
        }
    }

    // Put the pointer to the rotated miniCubes into the correct
    // positions of the global array
    if (s_interpolationValue == 0.0f || s_interpolationValue == 1.0f) {
        // Just reset, no rotation
        s_targetInterpolation = std::numeric_limits<float>::max();
        return;
    }
    if (s_targetInterpolation == 0.25f) {
        // Rotate the layer clockwise
        *s_correspondingGlobalPointers[0][0] = s_rotatingMiniCubes[0][2];
        *s_correspondingGlobalPointers[0][1] = s_rotatingMiniCubes[1][2];
        *s_correspondingGlobalPointers[0][2] = s_rotatingMiniCubes[2][2];
        *s_correspondingGlobalPointers[1][0] = s_rotatingMiniCubes[0][1];
        *s_correspondingGlobalPointers[1][2] = s_rotatingMiniCubes[2][1];
        *s_correspondingGlobalPointers[2][0] = s_rotatingMiniCubes[0][0];
        *s_correspondingGlobalPointers[2][1] = s_rotatingMiniCubes[1][0];
        *s_correspondingGlobalPointers[2][2] = s_rotatingMiniCubes[2][0];
    } else if (s_targetInterpolation == 0.75f) {
        // Rotate the layer counter clockwise
        *s_correspondingGlobalPointers[0][0] = s_rotatingMiniCubes[2][0];
        *s_correspondingGlobalPointers[0][1] = s_rotatingMiniCubes[1][0];
        *s_correspondingGlobalPointers[0][2] = s_rotatingMiniCubes[0][0];
        *s_correspondingGlobalPointers[1][0] = s_rotatingMiniCubes[2][1];
        *s_correspondingGlobalPointers[1][2] = s_rotatingMiniCubes[0][1];
        *s_correspondingGlobalPointers[2][0] = s_rotatingMiniCubes[2][2];
        *s_correspondingGlobalPointers[2][1] = s_rotatingMiniCubes[1][2];
        *s_correspondingGlobalPointers[2][2] = s_rotatingMiniCubes[0][2];
    } else { // Rotate by 180 degrees, more should not be possible
        *s_correspondingGlobalPointers[0][0] = s_rotatingMiniCubes[2][2];
        *s_correspondingGlobalPointers[0][1] = s_rotatingMiniCubes[2][1];
        *s_correspondingGlobalPointers[0][2] = s_rotatingMiniCubes[2][0];
        *s_correspondingGlobalPointers[1][0] = s_rotatingMiniCubes[1][2];
        *s_correspondingGlobalPointers[1][2] = s_rotatingMiniCubes[1][0];
        *s_correspondingGlobalPointers[2][0] = s_rotatingMiniCubes[0][2];
        *s_correspondingGlobalPointers[2][1] = s_rotatingMiniCubes[0][1];
        *s_correspondingGlobalPointers[2][2] = s_rotatingMiniCubes[0][0];
    }

    s_interpolationValue = 0.0f;
    s_targetInterpolation = std::numeric_limits<float>::max();
}

static inline void setTargetRotation(MiniCube& mc, Dimension dim) {
    mc.targetRotation = mc.startRotation + 2.0f * PI;
}

inline void setAnimation(int layer, Dimension dimension) {
    SCE_DBG_ALWAYS_ASSERT(layer < 3);

    s_rotatingDimension = dimension;

    // Origin of x/y always in top left corner of the slice, viewed from
    // front/top/left depending on dimension
    switch (dimension) {
    case DIM_X:
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                s_rotatingMiniCubes[x][y] = g_miniCubes[layer][y][2 - x];
                s_correspondingGlobalPointers[x][y] =
                    &g_miniCubes[layer][y][2 - x];
            }
        }
        break;
    case DIM_Y:
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                s_rotatingMiniCubes[x][y] = g_miniCubes[x][layer][2 - y];
                s_correspondingGlobalPointers[x][y] =
                    &g_miniCubes[x][layer][2 - y];
            }
        }
        break;
    case DIM_Z:
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                s_rotatingMiniCubes[x][y] = g_miniCubes[x][y][layer];
                s_correspondingGlobalPointers[x][y] = &g_miniCubes[x][y][layer];
            }
        }
        break;
    default:
        break;
    }

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            setTargetRotation(*s_rotatingMiniCubes[row][col], dimension);
        }
    }

    sceRtcGetCurrentTick(&s_lastTick);
}