#include "MiniCube.h"
#include "Helpers.h"

void renderMiniCube(const MiniCube &mc) {
    void *vertexDefaultBuffer;
    sceGxmReserveVertexDefaultUniformBuffer(s_context, &vertexDefaultBuffer);
    sceGxmSetUniformDataF(vertexDefaultBuffer, s_wvpParam, 0, 16,
                          (float *)&s_finalTransformation);
    sceGxmSetUniformDataF(vertexDefaultBuffer, s_rotParam, 0, 16,
                          (float *)&s_finalRotation);

    Matrix4 localToWorld;
    getLocalToWorldTransform(mc, localToWorld);
    sceGxmSetUniformDataF(vertexDefaultBuffer, s_localToWorldParam, 0, 16,
                          (float *)&localToWorld);

    /* draw the spinning triangle */
    sceGxmSetVertexStream(s_context, 0, mc.vertices);
    sceGxmDraw(s_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16,
               s_basicIndices, 6 * 6);
}

void getLocalToWorldTransform(const MiniCube &mc, Matrix4 &out) {
    out =
        Matrix4::translation(mc.position); // * Matrix4::rotation(mc.rotation);
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

MiniCube createMiniCube(Vector3 pos, int cubeLocation[3]) {
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

    // ToCopy
    return mc;
}

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
