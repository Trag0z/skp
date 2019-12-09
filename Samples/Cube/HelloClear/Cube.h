#include "MiniCube.h"

// Coordinate system used in this class is right handed. X is left, y is down, z
// is back
class Cube {
    SceGxmProgramParameter *m_localToWorldParam;
    float m_center[3];

    MiniCube *m_miniCube[3][3][3];
    MiniCube m_memBlock[3][3][3];

  public:
    enum Dimension { X = 0, Y = 1, Z = 2 };

    void init(float center[3], float miniCubeDiameter,
              SceGxmProgramParameter *miniCubePosParam);
    void render(SceGxmContext *context, void *vertexDefaultBuffer);
    void rotate(int layer, Cube::Dimension dimension, bool clockwise);
};