#pragma once
#include "MiniCube.h"
#include <gxt.h>

Vertex *MiniCube::s_vertices;
int32_t MiniCube::s_verticesUId;
uint16_t *MiniCube::s_indices;
int32_t MiniCube::s_indicesUId;
SceGxmTexture MiniCube::s_textures[7];

void MiniCube::init() {
    s_vertices =
        (Vertex *)graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
                                s_numVertices * sizeof(Vertex), 4,
                                SCE_GXM_MEMORY_ATTRIB_READ, &s_verticesUId);

    // What is allingment here? Why is it 2 in main.cpp?
    s_indices = (uint16_t *)graphicsAlloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 6 * 6 * sizeof(uint16_t),
        2, SCE_GXM_MEMORY_ATTRIB_READ, &s_indicesUId);

    // Vertices
    // Front
    s_vertices[0] = Vertex(-0.5f, -0.5f, -0.5f);
    s_vertices[1] = Vertex(0.5f, -0.5f, -0.5f);
    s_vertices[2] = Vertex(0.5f, 0.5f, -0.5f);
    s_vertices[3] = Vertex(-0.5f, 0.5f, -0.5f);
    // Back
    s_vertices[4] = Vertex(0.5f, -0.5f, 0.5f);
    s_vertices[5] = Vertex(-0.5f, -0.5f, 0.5f);
    s_vertices[6] = Vertex(0.5f, 0.5f, 0.5f);
    s_vertices[7] = Vertex(-0.5f, 0.5f, 0.5f);
    // Left
    s_vertices[8] = Vertex(-0.5f, -0.5f, 0.5f);
    s_vertices[9] = Vertex(-0.5f, -0.5f, -0.5f);
    s_vertices[10] = Vertex(-0.5f, 0.5f, -0.5f);
    s_vertices[11] = Vertex(-0.5f, 0.5f, 0.5f);
    // Right
    s_vertices[12] = Vertex(0.5f, -0.5f, -0.5f);
    s_vertices[13] = Vertex(0.5f, -0.5f, 0.5f);
    s_vertices[14] = Vertex(0.5f, 0.5f, 0.5f);
    s_vertices[15] = Vertex(0.5f, 0.5f, -0.5f);
    // Top
    s_vertices[16] = Vertex(-0.5f, 0.5f, 0.5f);
    s_vertices[17] = Vertex(0.5f, 0.5f, 0.5f);
    s_vertices[18] = Vertex(0.5f, 0.5f, -0.5f);
    s_vertices[19] = Vertex(-0.5f, 0.5f, -0.5f);
    // Bottom
    s_vertices[20] = Vertex(-0.5f, -0.5f, -0.5f);
    s_vertices[21] = Vertex(0.5f, -0.5f, -0.5f);
    s_vertices[22] = Vertex(0.5f, -0.5f, 0.5f);
    s_vertices[23] = Vertex(-0.5f, -0.5f, 0.5f);

    // Indics
    int count = 0;
    for (int side = 0; side < 6; ++side) {
        int baseIndex = side * 4;

        s_indices[count++] = baseIndex;
        s_indices[count++] = baseIndex + 1;
        s_indices[count++] = baseIndex + 2;

        s_indices[count++] = baseIndex + 2;
        s_indices[count++] = baseIndex + 3;
        s_indices[count++] = baseIndex;
    }

    // load textures
    // loadTexture(&s_textures[0], "app0:white.gxt");
    // loadTexture(&s_textures[1], "app0:black.gxt");
    // loadTexture(&s_textures[2], "app0:green.gxt");
    // loadTexture(&s_textures[3], "app0:orange.gxt");
    // loadTexture(&s_textures[4], "app0:blue.gxt");
    // loadTexture(&s_textures[5], "app0:red.gxt");
    // loadTexture(&s_textures[6], "app0:yellow.gxt");
}

void MiniCube::rotate(float radians, int axis) {
    assert(axis < 3);
    Quat rotation;
    if (axis == 0)
        rotation.rotationX(radians);
    else if (axis == 1)
        rotation.rotationY(radians);
    else
        rotation.rotationZ(radians);
    m_orientation += rotation;
    m_orientation = normalize(m_orientation);
    // m_rotation[axis] += degrees;
}

static uint32_t toRGB(TextureColor c) {
    switch (c) {
    case WHITE:
        return 0xffffff00;
    case BLACK:
        return 0x00000000;
    case GREEN:
        return 0x00ff0000;
    case ORANGE:
        return 0xff990000;
    case BLUE:
        return 0x0000ff00;
    case RED:
        return 0xff000000;
    case YELLOW:
        return 0xffff0000;
    default:
        return 0;
    }
}

static void setColors(MiniCube &mc, SceGxmTexture *textures, TextureColor front,
                      TextureColor back, TextureColor left, TextureColor right,
                      TextureColor top, TextureColor bottom) {
    TextureColor colors[6] = {front, back, left, right, top, bottom};
    // for (int side = 0; side < 6; ++side) {
    //     mc.textures()[side] = &textures[colors[side]];
    // }

    for (int side = 0; side < 6; ++side) {
        for (int vertex = 0; vertex < 4; ++vertex) {
            mc.m_vertices[side * 6 + vertex].color = toRGB(colors[side]);
        }
    }
};

void MiniCube::create(MiniCube &mc, const float position[3],
                      const int cubeLocation[3]) {
    mc.m_position[0] = position[0];
    mc.m_position[1] = position[1];
    mc.m_position[2] = position[2];

    mc.m_vertices =
        (Vertex *)graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
                                s_numVertices * sizeof(Vertex), 4,
                                SCE_GXM_MEMORY_ATTRIB_READ, &s_verticesUId);

    memcpy(mc.m_vertices, mc.s_vertices, sizeof(Vertex) * 6 * 4);

    if (cubeLocation[0] == 0) {     // X Left
        if (cubeLocation[1] == 0) { // Y Top
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, GREEN, BLACK, ORANGE,
                          BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, GREEN, BLACK, ORANGE,
                          BLACK);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, GREEN, BLACK, ORANGE,
                          BLACK);
            }
        } else if (cubeLocation[1] == 1) { // Y Middle
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, GREEN, BLACK, BLACK,
                          BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, GREEN, BLACK, BLACK,
                          BLACK);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, GREEN, BLACK, BLACK,
                          BLACK);
            }
        } else { // Y Bottom
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, GREEN, BLACK, BLACK,
                          RED);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, GREEN, BLACK, BLACK,
                          RED);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, GREEN, BLACK, BLACK,
                          RED);
            }
        }
    } else if (cubeLocation[0] == 1) { // X Middle
        if (cubeLocation[1] == 0) {    // Y Top
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, BLACK, BLACK, ORANGE,
                          BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, BLACK, BLACK, ORANGE,
                          BLACK);
            } else {
                setColors(mc, s_textures, BLACK, BLACK, BLACK, YELLOW, ORANGE,
                          BLACK);
            }
        } else if (cubeLocation[1] == 1) { // Y Middle
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, BLACK, BLACK, BLACK,
                          BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, BLACK, BLACK, BLACK,
                          BLACK);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, BLACK, BLACK, BLACK,
                          BLACK);
            }
        } else { // Y Bottom
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, BLACK, BLACK, BLACK,
                          RED);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, BLACK, BLACK, BLACK,
                          RED);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, BLACK, BLACK, BLACK,
                          RED);
            }
        }
    } else {                        // X Right
        if (cubeLocation[1] == 0) { // Y Top
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, BLACK, BLUE, ORANGE,
                          BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, BLACK, BLUE, ORANGE,
                          BLACK);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, BLACK, BLUE, ORANGE,
                          BLACK);
            }
        } else if (cubeLocation[1] == 1) { // Y Middle
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, BLACK, BLUE, BLACK,
                          BLACK);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, BLACK, BLUE, BLACK,
                          BLACK);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, BLACK, BLUE, BLACK,
                          BLACK);
            }
        } else { // Y Bottom
            if (cubeLocation[2] == 0) {
                setColors(mc, s_textures, WHITE, BLACK, BLACK, RED, BLACK, RED);
            } else if (cubeLocation[2] == 1) {
                setColors(mc, s_textures, BLACK, BLACK, BLACK, RED, BLACK, RED);
            } else {
                setColors(mc, s_textures, BLACK, YELLOW, BLACK, RED, BLACK,
                          RED);
            }
        }
    }
}