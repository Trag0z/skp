#pragma once
#include "Globals.h"
#include "Input.h"
#include "System.h"
#include <rtc.h>
#include <sce_geometry.h>
#include <touch.h>

extern Matrix4 g_finalTransformation;
extern Matrix4 g_finalRotation;
extern const SceGxmProgramParameter* g_wvpParam;
extern const SceGxmProgramParameter* g_localToWorldParam;
extern AnimationState g_animationState;
extern const float g_miniCubeHalfSize;

static MiniCube* s_miniCubes;

static void initCube();

static void update();

int main(void) {
    int returnCode = SCE_OK;

    /* initialize libdbgfont and libgxm */
    returnCode = initGxm();
    SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);

    SceDbgFontConfig config;
    memset(&config, 0, sizeof(SceDbgFontConfig));
    config.fontSize = SCE_DBGFONT_FONTSIZE_LARGE;

    returnCode = sceDbgFontInit(&config);
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
        render(s_miniCubes);
        sceDbgFontPrint(20, 20, 0xffffffff, "Hello World");
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

void update(void) {

    processFrontTouch(s_miniCubes);
    progressAnimations(s_miniCubes);

    g_finalRotation = Matrix4::rotationZYX(processBackTouch());
    Matrix4 lookAt =
        Matrix4::lookAt(Point3(0.0f, 0.0f, -7.0f), Point3(0.0f, 0.0f, 0.0f),
                        Vector3(0.0f, -1.0f, 0.0f));
    Matrix4 perspective = Matrix4::perspective(
        3.141592f / 4.0f, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1f,
        10.0f);

    g_finalTransformation = perspective * lookAt * g_finalRotation;
}

static void initCube() {
    s_miniCubes = new MiniCube[27];
    int i = 0;
    for (int z = 0; z < 3; ++z) {
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                int cubeLocation[3] = {x, y, z};
                s_miniCubes[i++] = createMiniCube(
                    Vector3(static_cast<float>(x - 1) *
                                (g_miniCubeHalfSize * 2 + 0.01f),
                            static_cast<float>(y - 1) *
                                (g_miniCubeHalfSize * 2 + 0.01f),
                            static_cast<float>(z - 1) *
                                (g_miniCubeHalfSize * 2 + 0.01f)),
                    cubeLocation);
            }
        }
    }
}
