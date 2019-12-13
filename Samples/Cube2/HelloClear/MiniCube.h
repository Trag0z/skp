#pragma once
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;

const static Vertex s_defaultVertices[24];

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

struct MiniCube {
    Vertex *vertices;
    Vector3 position;
    Vector3 rotation;

    int32_t verticesUId;
};

inline void renderMiniCube(const MiniCube &mc);

inline void getLocalToWorldTransform(const MiniCube &mc, Matrix4 &out);

static void setColors(MiniCube &mc, Color front, Color back, Color left,
                      Color right, Color top, Color bottom);

inline MiniCube createMiniCube(Vector3 pos, int cubeLocation[3]);
