#pragma once
#include "System.h"

//!! The program parameter for the transformation of the triangle
static Matrix4 s_finalTransformation;
extern const SceGxmProgramParameter *s_wvpParam = NULL;

// The rotation information.
static Matrix4 s_finalRotation;
extern const SceGxmProgramParameter *s_rotParam = NULL;

// New stuff for MiniCube
extern const SceGxmProgramParameter *s_localToWorldParam = NULL;

static MiniCube *s_miniCubes;
static float s_accumulatedTurningAngleX;
static float s_accumulatedTurningAngleY;

// /* Callback function to allocate memory for the shader patcher */
// static void *patcherHostAlloc(void *userData, uint32_t size);

// /* Callback function to allocate memory for the shader patcher */
// static void patcherHostFree(void *userData, void *mem);

// /*	Callback function for displaying a buffer */
// static void displayCallback(const void *callbackData);

// /*	Helper function to allocate memory and map it for the GPU */
// static void *graphicsAlloc(SceKernelMemBlockType type, uint32_t size,
//                            uint32_t alignment, uint32_t attribs, SceUID
//                            *uid);

// /*	Helper function to free memory mapped to the GPU */
// static void graphicsFree(SceUID uid);

// /* Helper function to allocate memory and map it as vertex USSE code for the
// GPU
//  */
// static void *vertexUsseAlloc(uint32_t size, SceUID *uid, uint32_t
// *usseOffset);

// /* Helper function to free memory mapped as vertex USSE code for the GPU */
// static void vertexUsseFree(SceUID uid);

// /* Helper function to allocate memory and map it as fragment USSE code for
// the
//  * GPU */
// static void *fragmentUsseAlloc(uint32_t size, SceUID *uid,
//                                uint32_t *usseOffset);

// /* Helper function to free memory mapped as fragment USSE code for the GPU */
// static void fragmentUsseFree(SceUID uid);

// /*	@brief Main entry point for the application
//         @return Error code result of processing during execution: <c> SCE_OK
//    </c> on success, or another code depending upon the error
// */
// int main(void);

// // !! Here we create the matrix.
// void Update(void);

// /*	@brief Initializes the graphics services and the libgxm graphics library
//         @return Error code result of processing during execution: <c> SCE_OK
//    </c> on success, or another code depending upon the error
// */
// static int initGxm(void);

// /*	 @brief Creates scenes with libgxm */
// static void createGxmData(void);

// /*	@brief Main rendering function to draw graphics to the display */
// static void render(void);

// /*	@brief render libgxm scenes */
// static void renderGxm(void);

// /*	@brief cycle display buffer */
// static void cycleDisplayBuffers(void);

// /*	@brief Destroy scenes with libgxm */
// static void destroyGxmData(void);

// /*	@brief Function to shut down libgxm and the graphics display services
//         @return Error code result of processing during execution: <c> SCE_OK
//    </c> on success, or another code depending upon the error
// */
// static int shutdownGxm(void);

static void initCube();

/* Main entry point of program */
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
        Update();
        render();
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

void Update(void) {

    SceCtrlData result;
    sceCtrlReadBufferPositive(0, &result, 1);

    s_accumulatedTurningAngleX += makeFloat(result.lx) * 0.01f;
    s_accumulatedTurningAngleY += makeFloat(result.ly) * 0.01f;

    s_finalRotation = Matrix4::rotationZYX(
        Vector3(s_accumulatedTurningAngleY, -s_accumulatedTurningAngleX, 0.0f));
    // ToCopy
    Matrix4 lookAt =
        Matrix4::lookAt(Point3(0.0f, 0.0f, -10.0f), Point3(0.0f, 0.0f, 10.0f),
                        Vector3(0.0f, -1.0f, 0.0f));
    Matrix4 perspective = Matrix4::perspective(
        3.141592f / 4.0f, (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT, 0.1f,
        10.0f);

    s_finalTransformation = perspective * lookAt * s_finalRotation;
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
