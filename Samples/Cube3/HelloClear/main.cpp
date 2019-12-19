#pragma once
#include "Globals.h"
#include "Input.h"
#include "System.h"
#include <gxt.h>
#include <rtc.h>
#include <sce_geometry.h>
#include <touch.h>

extern Matrix4 g_finalTransformation;
extern Matrix4 g_finalRotation;
extern const SceGxmProgramParameter* g_wvpParam;
extern const SceGxmProgramParameter* g_localToWorldParam;
extern AnimationState g_animationState;
extern const float g_miniCubeHalfSize;
extern MiniCube* g_miniCubes[3][3][3];
extern SceGxmTexture g_texture;

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

static Quat m_orientationQuaternion(0.0f, 0.0f, 0.0f, 1.0f);

void update(void) {

    processFrontTouch();
    progressAnimations();

    Vector2 backTouchMove = processBackTouch();

    Quat rotationVelocity(backTouchMove.getX().getAsFloat(),
                          backTouchMove.getY().getAsFloat(), 0.0f, 0.0f);

    m_orientationQuaternion +=
        0.5f * rotationVelocity * m_orientationQuaternion;
    m_orientationQuaternion = normalize(m_orientationQuaternion);

    Matrix4 lookAt =
        Matrix4::lookAt(Point3(0.0f, 0.0f, -7.0f), Point3(0.0f, 0.0f, 0.0f),
                        Vector3(0.0f, -1.0f, 0.0f));
    Matrix4 perspective = Matrix4::perspective(
        3.141592f / 4.0f, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1f,
        10.0f);

    g_finalTransformation =
        perspective * lookAt * Matrix4::rotation(m_orientationQuaternion);
}

void loadTexture(SceGxmTexture* texture, const char* filename) {
    SceUID fileID = sceIoOpen(filename, SCE_O_RDONLY, SCE_STM_RU);
    unsigned int fileSize = (unsigned int)sceIoLseek(fileID, 0, SCE_SEEK_END);
    sceIoLseek(fileID, 0, SCE_SEEK_SET);

    // Why this randomly allocated memory that never gets freed?
    void* gxt = malloc(fileSize);
    SceSSize bytesRead = sceIoRead(fileID, gxt, fileSize);
    SCE_DBG_ALWAYS_ASSERT(bytesRead == fileSize);
    SCE_DBG_ALWAYS_ASSERT(sceGxtCheckData(gxt) == SCE_OK);

    const void* dataSrc = sceGxtGetDataAddress(gxt);
    const uint32_t dataSize = sceGxtGetDataSize(gxt);
    SceUID texID;
    void* texPtr = graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
                                 dataSize, SCE_GXM_TEXTURE_ALIGNMENT,
                                 SCE_GXM_MEMORY_ATTRIB_READ, &texID);

    std::memcpy(texPtr, dataSrc, dataSize);
    SceGxtErrorCode returnCode = sceGxtInitTexture(texture, gxt, dataSrc, 0);
    SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);

	sceIoClose(fileID);
}

static void initCube() {
    loadTexture(&g_texture, "app0:green.gxt");

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
