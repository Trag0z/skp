#pragma once
#include "MiniCube.h"
#include <gxt.h>

void MiniCube::init() {
    s_vertices =
        (Vertex *)graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
                                s_numVertices * sizeof(Vertex), 3,
                                SCE_GXM_MEMORY_ATTRIB_READ, &s_verticesUId);

    // What is allingment here? Why is it 2 in main.cpp?
    s_indeces = (uint16_t *)graphicsAlloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 6 * 6 * sizeof(uint16_t),
        2, SCE_GXM_MEMORY_ATTRIB_READ, &s_indicesUId);

    // Vertices
    // Front
    s_vertices[0] = {{-0.5f, -0.5f, -0.5f}};
    s_vertices[1] = {{0.5f, -0.5f, -0.5f}};
    s_vertices[2] = {{0.5f, 0.5f, -0.5f}};
    s_vertices[3] = {{-0.5f, 0.5f, -0.5f}};
    // Back
    s_vertices[4] = {{0.5f, -0.5f, 0.5f}};
    s_vertices[5] = {{-0.5f, -0.5f, 0.5f}};
    s_vertices[6] = {{0.5f, 0.5f, 0.5f}};
    s_vertices[7] = {{-0.5f, 0.5f, 0.5f}};
    // Left
    s_vertices[8] = {{-0.5f, -0.5f, 0.5f}};
    s_vertices[9] = {{-0.5f, -0.5f, -0.5f}};
    s_vertices[10] = {{-0.5f, 0.5f, -0.5f}};
    s_vertices[11] = {{-0.5f, 0.5f, 0.5f}};
    // Right
    s_vertices[12] = {{0.5f, -0.5f, -0.5f}};
    s_vertices[13] = {{0.5f, -0.5f, 0.5f}};
    s_vertices[14] = {{0.5f, 0.5f, 0.5f}};
    s_vertices[15] = {{0.5f, 0.5f, -0.5f}};
    // Top
    s_vertices[16] = {{-0.5f, 0.5f, 0.5f}};
    s_vertices[17] = {{0.5f, 0.5f, 0.5f}};
    s_vertices[18] = {{0.5f, 0.5f, -0.5f}};
    s_vertices[19] = {{-0.5f, 0.5f, -0.5f}};
    // Bottom
    s_vertices[20] = {{-0.5f, -0.5f, -0.5f}};
    s_vertices[21] = {{0.5f, -0.5f, -0.5f}};
    s_vertices[22] = {{0.5f, -0.5f, 0.5f}};
    s_vertices[23] = {{-0.5f, -0.5f, 0.5f}};

    // Indics
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

    // load textures
    auto loadTexture = [](SceGxmTexture *texture, const char *filename) {
        // NOTE: Dangling file pointers here?
        SceUID fileID = sceIoOpen(filename, SCE_O_RDONLY, SCE_STM_RU);
        SceOff fileSize = sceIoLseek(fileID, 0, SCE_SEEK_END);
        sceIoLseek(fileID, 0, SCE_SEEK_SET);

        // Why this randomly allocated memory that never gets freed?
        void *dummyMem = malloc(fileSize);

        const void *dataSrc = sceGxtGetDataAddress(dummyMem);
        const uint32_t dataSize = sceGxtGetDataSize(dummyMem);
        SceUID texID;
        void *texPtr = graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
                                     dataSize, SCE_GXM_TEXTURE_ALIGNMENT,
                                     SCE_GXM_MEMORY_ATTRIB_READ, &texID);

        memcpy(texPtr, dataSrc, dataSize);
        sceGxtInitTexture(&texture, texPtr, dataSrc, 0);
    };

    loadTexture(&s_textures[0], "app0:white.gxt");
    loadTexture(&s_textures[1], "app0:black.gxt");
    loadTexture(&s_textures[2], "app0:green.gxt");
    loadTexture(&s_textures[3], "app0:orange.gxt");
    loadTexture(&s_textures[4], "app0:blue.gxt");
    loadTexture(&s_textures[5], "app0:red.gxt");
    loadTexture(&s_textures[6], "app0:yellow.gxt");
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

void MiniCube::create(MiniCube &mc, const float position[3],
                      const int cubeLocation[3]) {
    mc.m_position[0] = position[0];
    mc.m_position[1] = position[1];
    mc.m_position[2] = position[2];

    auto setColors = [&mc](TextureColors front, TextureColors back,
                           TextureColors left, TextureColors right,
                           TextureColors top, TextureColors bottom) {
        TextureColors colors[6] = {front, back, left, right, top, bottom};
        for (int side = 0; side < 6; ++side) {
            mc.m_textures[side] = s_textures[colors[side]];
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