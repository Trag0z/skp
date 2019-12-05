#include "Cube.h"
#include <assert.h>

void Cube::init(float center[3], float miniCubeDiameter) {
    m_center[0] = center[0];
    m_center[1] = center[1];
    m_center[2] = center[2];

    float firstCubePos[3] = {m_center[0] - miniCubeDiameter,
                             m_center[1] - miniCubeDiameter,
                             m_center[2] - miniCubeDiameter};

    MiniCube::init();

    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            for (int z = 0; z < 3; ++z) {
                m_miniCube[x][y][z] = &m_memBlock[x][y][z];
                float pos[3] = {firstCubePos[0] + x * miniCubeDiameter,
                                firstCubePos[1] + y * miniCubeDiameter,
                                firstCubePos[2] + z * miniCubeDiameter};
                MiniCube::create(m_memBlock[x][y][z], pos);
            }
        }
    }
}

void Cube::rotate(int layer, Cube::Dimension dimension, bool clockwise) {
    assert(layer < 3);

    MiniCube chosenLayer[3][3];
    MiniCube rotatedLayer[3][3];
    switch (dimension) {
    case Dimension::X:
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                chosenLayer[i][j] = *m_miniCube[layer][i][j];
            }
        }
        if (clockwise) {
            rotatedLayer[0][0] = chosenLayer[0][2];
            rotatedLayer[0][1] = chosenLayer[1][2];
            rotatedLayer[0][2] = chosenLayer[2][2];
            rotatedLayer[1][0] = chosenLayer[0][1];
            rotatedLayer[1][1] = chosenLayer[1][1];
            rotatedLayer[1][2] = chosenLayer[2][1];
            rotatedLayer[2][0] = chosenLayer[0][0];
            rotatedLayer[2][1] = chosenLayer[1][0];
            rotatedLayer[2][2] = chosenLayer[2][0];
        } else {
            rotatedLayer[0][0] = chosenLayer[2][0];
            rotatedLayer[0][1] = chosenLayer[1][0];
            rotatedLayer[0][2] = chosenLayer[0][0];
            rotatedLayer[1][0] = chosenLayer[1][2];
            rotatedLayer[1][1] = chosenLayer[1][1];
            rotatedLayer[1][2] = chosenLayer[0][1];
            rotatedLayer[2][0] = chosenLayer[2][2];
            rotatedLayer[2][1] = chosenLayer[1][2];
            rotatedLayer[2][2] = chosenLayer[0][2];
        }
        break;
    case Dimension::Y:
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                chosenLayer[i][j] = *m_miniCube[i][layer][j];
            }
        }
        if (!clockwise) {
            rotatedLayer[0][0] = chosenLayer[0][2];
            rotatedLayer[0][1] = chosenLayer[1][2];
            rotatedLayer[0][2] = chosenLayer[2][2];
            rotatedLayer[1][0] = chosenLayer[0][1];
            rotatedLayer[1][1] = chosenLayer[1][1];
            rotatedLayer[1][2] = chosenLayer[2][1];
            rotatedLayer[2][0] = chosenLayer[0][0];
            rotatedLayer[2][1] = chosenLayer[1][0];
            rotatedLayer[2][2] = chosenLayer[2][0];
        } else {
            rotatedLayer[0][0] = chosenLayer[2][0];
            rotatedLayer[0][1] = chosenLayer[1][0];
            rotatedLayer[0][2] = chosenLayer[0][0];
            rotatedLayer[1][0] = chosenLayer[1][2];
            rotatedLayer[1][1] = chosenLayer[1][1];
            rotatedLayer[1][2] = chosenLayer[0][1];
            rotatedLayer[2][0] = chosenLayer[2][2];
            rotatedLayer[2][1] = chosenLayer[1][2];
            rotatedLayer[2][2] = chosenLayer[0][2];
        }
        break;
    case Dimension::Z:
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                chosenLayer[i][j] = *m_miniCube[i][j][layer];
            }
        }
        if (clockwise) {
            rotatedLayer[0][0] = chosenLayer[0][2];
            rotatedLayer[0][1] = chosenLayer[1][2];
            rotatedLayer[0][2] = chosenLayer[2][2];
            rotatedLayer[1][0] = chosenLayer[0][1];
            rotatedLayer[1][1] = chosenLayer[1][1];
            rotatedLayer[1][2] = chosenLayer[2][1];
            rotatedLayer[2][0] = chosenLayer[0][0];
            rotatedLayer[2][1] = chosenLayer[1][0];
            rotatedLayer[2][2] = chosenLayer[2][0];
        } else {
            rotatedLayer[0][0] = chosenLayer[2][0];
            rotatedLayer[0][1] = chosenLayer[1][0];
            rotatedLayer[0][2] = chosenLayer[0][0];
            rotatedLayer[1][0] = chosenLayer[2][1];
            rotatedLayer[1][1] = chosenLayer[1][1];
            rotatedLayer[1][2] = chosenLayer[0][1];
            rotatedLayer[2][0] = chosenLayer[2][2];
            rotatedLayer[2][1] = chosenLayer[1][2];
            rotatedLayer[2][2] = chosenLayer[0][2];
        }
        break;
    default:
        break;
    }

    if (clockwise) {
        // NOTE: does this run through all 9 cubes?
        for (auto &row : rotatedLayer) {
            for (auto &col : row) {
                col.rotate(90.0f, static_cast<size_t>(dimension));
            }
        }
    }
}