#pragma once
#include "Globals.h"
#include "Input.h"
#include "System.h"
#include <rtc.h>
#include <sce_geometry.h>
#include <touch.h>

extern Matrix4 g_cameraTransformation;
extern Matrix4 g_finalRotation;
extern const SceGxmProgramParameter* g_wvpParam;
extern const SceGxmProgramParameter* g_localToWorldParam;
extern AnimationState g_animationState;
extern const float g_miniCubeHalfSize;
extern MiniCube* g_miniCubes[3][3][3];

static MiniCube* s_miniCubes;
static Quat s_cameraOrientation = Quat::identity();

static void initCube();

static void update();

int main(void) {
    int returnCode = SCE_OK;

    /* initialize libdbgfont and libgxm */
    returnCode = initGxm();
    SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);

    /* Message for SDK sample auto test */
    printf("## simple: INIT SUCCEEDED ##\n");

    /* create gxm graphics data */
    createGxmData();

    // Set sampling mode for input device.
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALANALOG_WIDE);

    initTouch();
    initCube();

    /* 6. main loop */
    while (true) {
        update();
        render();
        cycleDisplayBuffers();
    }

    /*
        // 10. wait until rendering is done
        sceGxmFinish( s_context );

        // destroy gxm graphics data
        destroyGxmData();

        // shutdown libdbgfont and libgxm
        returnCode = shutdownGxm();
        SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

        // Message for SDK sample auto test
        printf( "## api_libdbgfont/simple: FINISHED ##\n" );

        return returnCode;
    */
}

float makeFloat(unsigned char input) {
    return (((float)(input)) / 255.0f * 2.0f) - 1.0f;
}

static Quat s_orientation = Quat::identity();

void update(void) {

    processFrontTouch();
    progressAnimations();

    Vector2 backTouchMove = processBackTouch();

    Quat addedRotation =
        Quat::rotation(Vector3(backTouchMove.getY().getAsFloat() * 0.01f,
                               backTouchMove.getX().getAsFloat() * 0.01f, 0.0f),
                       sce::Vectormath::Simd::kXYZ);

    s_orientation = normalize(addedRotation * s_orientation);

    Matrix4 lookAt =
        Matrix4::lookAt(Point3(0.0f, 0.0f, -7.0f), Point3(0.0f, 0.0f, 0.0f),
                        Vector3(0.0f, -1.0f, 0.0f));
    Matrix4 perspective = Matrix4::perspective(
        3.141592f / 4.0f, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1f,
        10.0f);

    g_cameraTransformation =
        perspective * lookAt * Matrix4::rotation(s_orientation);
}

static void initCube() {
    s_miniCubes = new MiniCube[27];
    int i = 0;
    for (int z = 0; z < 3; ++z) {
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                int cubeLocation[3] = {x, y, z};
                s_miniCubes[i] = createMiniCube(
                    Vector3(static_cast<float>(x - 1) *
                                (g_miniCubeHalfSize + 0.01f) * 2.0f,
                            static_cast<float>(y - 1) *
                                (g_miniCubeHalfSize + 0.01f) * 2.0f,
                            static_cast<float>(z - 1) *
                                (g_miniCubeHalfSize + 0.01f) * 2.0f),
                    cubeLocation);
                g_miniCubes[x][y][z] = &s_miniCubes[i];
                ++i;
            }
        }
    }
}
