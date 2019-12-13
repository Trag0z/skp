#pragma once
#include "System.h"

Matrix4 g_finalTransformation = Matrix4();
Matrix4 g_finalRotation = Matrix4();
const SceGxmProgramParameter *g_wvpParam = NULL;
const SceGxmProgramParameter *g_rotParam = NULL;
const SceGxmProgramParameter *g_localToWorldParam = NULL;

static MiniCube *s_miniCubes;
static float s_accumulatedTurningAngleX;
static float s_accumulatedTurningAngleY;

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

    SceCtrlData result;
    sceCtrlReadBufferPositive(0, &result, 1);

    s_accumulatedTurningAngleX += makeFloat(result.lx) * 0.01f;
    s_accumulatedTurningAngleY += makeFloat(result.ly) * 0.01f;

    g_finalRotation = Matrix4::rotationZYX(
        Vector3(s_accumulatedTurningAngleY, -s_accumulatedTurningAngleX, 0.0f));
    // ToCopy
    Matrix4 lookAt =
        Matrix4::lookAt(Point3(0.0f, 0.0f, -10.0f), Point3(0.0f, 0.0f, 1.0f),
                        Vector3(0.0f, -1.0f, 0.0f));
    Matrix4 perspective = Matrix4::perspective(
        3.141592f / 4.0f, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1f,
        20.0f);

    g_finalTransformation = perspective * lookAt * g_finalRotation;
};

static void initCube() {
    s_miniCubes = new MiniCube[27];
    int i = 0;
    for (int z = 0; z < 3; ++z) {
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                int cubeLocation[3] = {x, y, z};
                s_miniCubes[i++] =
                    createMiniCube(Vector3(static_cast<float>(x - 1) * 1.01f,
                                           static_cast<float>(y - 1) * 1.01f,
                                           static_cast<float>(z - 1) * 1.01f),
                                   cubeLocation);
            }
        }
    }
}
