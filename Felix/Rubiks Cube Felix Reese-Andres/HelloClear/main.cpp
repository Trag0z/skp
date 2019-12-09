/* SCE CONFIDENTIAL
 * PlayStation(R)Vita Programmer Tool Runtime Library Release 02.000.081
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

// All fonts related stuff has been ripped out.

/*	

	This sample shows how to initialize libdbgfont (and libgxm),
	and render debug font with triangle for clear the screen.

	This sample is split into the following sections:

		1. Initialize libdbgfont
		2. Initialize libgxm
		3. Allocate display buffers, set up the display queue
		4. Create a shader patcher and register programs
		5. Create the programs and data for the clear
		6. Start the main loop
			7. Update step
			8. Rendering step
			9. Flip operation and render debug font at display callback
		10. Wait for rendering to complete
		11. Destroy the programs and data for the clear triangle
		12. Finalize libgxm
		13. Finalize libdbgfont

	Please refer to the individual comment blocks for details of each section.
*/

#include "CubeLogic.h"
#include "InputManager.h"
#include "DebugUpdate.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sceerror.h>

#include <gxm.h>
#include <kernel.h>
#include <ctrl.h>
#include <display.h>
#include <libdbg.h>

#include <libdbgfont.h>

#include <math.h>
#include <vectormath.h>

#include<libsysmodule.h>
#include<razor_capture.h>
#include<libperf.h>

#include<sce_geometry.h>

#include <assert.h>

#include<touch.h>

using namespace sce::Vectormath::Simd::Aos;		//um Aos zu nutzen!!!
using namespace sce::Geometry::Aos;

/*	Define the debug font pixel color format to render to. */
#define DBGFONT_PIXEL_FORMAT		SCE_DBGFONT_PIXELFORMAT_A8B8G8R8


/*	Define the width and height to render at the native resolution */
#define DISPLAY_WIDTH				960
#define DISPLAY_HEIGHT				544
#define DISPLAY_STRIDE_IN_PIXELS	1024	//wie die einzelnen Pixel im bildschirm abgelegt ist, nächstgrößte 2erPotenz über 960, eine Rasterzeile, die letzten 64 Pixel sind randstreifen, werden nicht angezeigt


/*	Define the libgxm color format to render to.
	This should be kept in sync with the display format to use with the SceDisplay library.
*/
#define DISPLAY_COLOR_FORMAT		SCE_GXM_COLOR_FORMAT_A8B8G8R8
#define DISPLAY_PIXEL_FORMAT		SCE_DISPLAY_PIXELFORMAT_A8B8G8R8	//8Bit, keine fließkomma also

/*	Define the number of back buffers to use with this sample.  Most applications
	should use a value of 2 (double buffering) or 3 (triple buffering).
*/
#define DISPLAY_BUFFER_COUNT	3

/*	Define the maximum number of queued swaps that the display queue will allow.
	This limits the number of frames that the CPU can get ahead of the GPU,
	and is independent of the actual number of back buffers.  The display
	queue will block during sceGxmDisplayQueueAddEntry if this number of swaps
	have already been queued.
*/
#define DISPLAY_MAX_PENDING_SWAPS	2	//wie weit kann die cpu vor der gpu her rennen, je mehr frames ich queuen kann, desto flexibler kann ich auf frame range schwankungen reagieren, aber nachteil wenn höher
										//wie viele swaps werden zwischengespeichert


/*	Helper macro to align a value */
#define ALIGN(x, a)					(((x) + ((a) - 1)) & ~((a) - 1))	//hilfsmakro


/*	The build process for the sample embeds the shader programs directly into the
	executable using the symbols below.  This is purely for convenience, it is
	equivalent to simply load the binary file into memory and cast the contents
	to type SceGxmProgram.
*/
extern const SceGxmProgram binaryClearVGxpStart;										//wo im speicher fangen die shader an
extern const SceGxmProgram binaryClearFGxpStart;										//

/*	Data structure for clear geometry */
typedef struct ClearVertex																//was in den vertex shader hineinkommt, Werte in normalized d evice coordinates
{
	float x;
	float y;

} ClearVertex;


// !! Data related to rendering vertex.
extern const SceGxmProgram binaryBasicVGxpStart;
extern const SceGxmProgram binaryBasicFGxpStart;


/*	Data structure for basic geometry */
typedef struct BasicVertex
{
	float position[3];		//easier to index
	float normal[3];		//die Normale für die Farbe

							/*uint32 bit wert als Farbe, is so aufgebaut, dass man in RGB je 8 Bit hat*/
	uint32_t color;			// Data gets expanded to float 4 in vertex shader.
							//evtl auch integer 0-6 wählen für farbe

	int ANIMATION_FLAG;

} BasicVertex;


/*	Data structure to pass through the display queue.  This structure is
	serialized during sceGxmDisplayQueueAddEntry, and is used to pass
	arbitrary data to the display callback function, called from an internal
	thread once the back buffer is ready to be displayed.

	In this example, we only need to pass the base address of the buffer.
*/
typedef struct DisplayData
{
	void *address;
} DisplayData;

static SceGxmContextParams		s_contextParams;			/* libgxm context parameter */
static SceGxmRenderTargetParams s_renderTargetParams;		/* libgxm render target parameter */
static SceGxmContext			*s_context			= NULL;	/* libgxm context */
static SceGxmRenderTarget		*s_renderTarget		= NULL;	/* libgxm render target */
static SceGxmShaderPatcher		*s_shaderPatcher	= NULL;	/* libgxm shader patcher */

/*	display data */
static void							*s_displayBufferData[ DISPLAY_BUFFER_COUNT ];	//speicherbereich wo das display liegt?!
static SceGxmSyncObject				*s_displayBufferSync[ DISPLAY_BUFFER_COUNT ];
static int32_t						s_displayBufferUId[ DISPLAY_BUFFER_COUNT ];
static SceGxmColorSurface			s_displaySurface[ DISPLAY_BUFFER_COUNT ];
static uint32_t						s_displayFrontBufferIndex = 0;
static uint32_t						s_displayBackBufferIndex = 0;
static SceGxmDepthStencilSurface	s_depthSurface;									//tiefenpuffer

/*	shader data */
static int32_t					s_clearVerticesUId;
static int32_t					s_clearIndicesUId;
static SceGxmShaderPatcherId	s_clearVertexProgramId;
static SceGxmShaderPatcherId	s_clearFragmentProgramId;
// !! Shader pactcher addded.									//für die neuen basic shader
static SceGxmShaderPatcherId	s_basicVertexProgramId;
static SceGxmShaderPatcherId	s_basicFragmentProgramId;
static SceUID					s_patcherFragmentUsseUId;
static SceUID					s_patcherVertexUsseUId;
static SceUID					s_patcherBufferUId;
static SceUID					s_depthBufferUId;
static SceUID					s_vdmRingBufferUId;
static SceUID					s_vertexRingBufferUId;
static SceUID					s_fragmentRingBufferUId;
static SceUID					s_fragmentUsseRingBufferUId;
static ClearVertex				*s_clearVertices			= NULL;
static uint16_t					*s_clearIndices				= NULL;
static SceGxmVertexProgram		*s_clearVertexProgram		= NULL;
static SceGxmFragmentProgram	*s_clearFragmentProgram		= NULL;
// !! Data added.
static SceGxmVertexProgram		*s_basicVertexProgram		= NULL;
static SceGxmFragmentProgram	*s_basicFragmentProgram		= NULL;
static BasicVertex				*s_basicVertices			= NULL;
static uint16_t					*s_basicIndices				= NULL;
static int32_t					s_basicVerticesUId;
static int32_t					s_basicIndiceUId;


//!! The program parameter for the transformation of the Cube

void CreateCubeSideTile(BasicVertex* field, int type, int direction, int x, int y);			

static Matrix4 s_finalTransformation;										//Transformationsmatrix
static const SceGxmProgramParameter *s_wvpParam = NULL;	

static Matrix4 s_finalRotation;												//Transformationsmatrix für die Normalen, also für die Farbe, um von object in world coordinates zu transformieren
static const SceGxmProgramParameter *s_rotParam = NULL;

static Matrix4 s_animationRotation;											//AnimationsMatrix
//static const SceGxmProgramParameter *s_animParam = NULL;

static const SceGxmProgramParameter *s_rotAngleParam = NULL;


//von folgenden Funktionen erklären können was sie tun!!!

/* Callback function to allocate memory for the shader patcher */
static void *patcherHostAlloc( void *userData, uint32_t size );

/* Callback function to allocate memory for the shader patcher */
static void patcherHostFree( void *userData, void *mem );

/*	Callback function for displaying a buffer */
static void displayCallback( const void *callbackData );

/*	Helper function to allocate memory and map it for the GPU */
static void *graphicsAlloc( SceKernelMemBlockType type, uint32_t size, uint32_t alignment, uint32_t attribs, SceUID *uid );

/*	Helper function to free memory mapped to the GPU */
static void graphicsFree( SceUID uid );

/* Helper function to allocate memory and map it as vertex USSE code for the GPU */
static void *vertexUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset );

/* Helper function to free memory mapped as vertex USSE code for the GPU */
static void vertexUsseFree( SceUID uid );

/* Helper function to allocate memory and map it as fragment USSE code for the GPU */
static void *fragmentUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset );

/* Helper function to free memory mapped as fragment USSE code for the GPU */
static void fragmentUsseFree( SceUID uid );


/*	@brief Main entry point for the application
	@return Error code result of processing during execution: <c> SCE_OK </c> on success,
	or another code depending upon the error
*/
int main( void );


// !! Here we create the matrix.
void update(void);

/*	@brief Initializes the graphics services and the libgxm graphics library
	@return Error code result of processing during execution: <c> SCE_OK </c> on success,
	or another code depending upon the error
*/
static int initGxm( void );

/*	 @brief Creates scenes with libgxm */
static void createGxmData( void );


/*	@brief Main rendering function to draw graphics to the display */
static void render( void );

/*	@brief render libgxm scenes */
static void renderGxm( void );

/*	@brief cycle display buffer */
static void cycleDisplayBuffers( void );

/*	@brief Destroy scenes with libgxm */
static void destroyGxmData( void );

/*	@brief Function to shut down libgxm and the graphics display services
	@return Error code result of processing during execution: <c> SCE_OK </c> on success,
	or another code depending upon the error
*/
static int shutdownGxm( void );

void colorizeVertices(BasicVertex* vertices);
void setAnimationFlagSide(BasicVertex* vertices, int side, int flag);
void setAnimationFlag(BasicVertex* vertices, int dimension, int slice);
void resetAnimationFlag(BasicVertex* vertices);
void CreateCubeSideTile(BasicVertex* field, int type, int direction, int x, int y);

void checkFrontTouch();
void checkBackTouch();
void doButtonAnimation();
void checkButtonInput();
void calculateFinalTransformation();


/*	@brief User main thread parameters */
extern const char			sceUserMainThreadName[]		= "simple_main_thr";
extern const int			sceUserMainThreadPriority	= SCE_KERNEL_DEFAULT_PRIORITY_USER;
extern const unsigned int	sceUserMainThreadStackSize	= SCE_KERNEL_STACK_SIZE_DEFAULT_USER_MAIN;

/*	@brief libc parameters */
unsigned int	sceLibcHeapSize	= 1*1024*1024;		//1MB speicher festlegen


CubeLogic* cube;
InputManager* input;

/* Main entry point of program */
int main( void )
{

	CubeLogic cubeLogic;
	cube = &cubeLogic;

	InputManager inputManager;
	input = &inputManager;

	input->buttonPressed = false;

	int returnCode = SCE_OK;

	returnCode = sceSysmoduleLoadModule(SCE_SYSMODULE_PERF);		
	sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
	//sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_HUD);				//HUD anzeigen

	/* initialize libdbgfont and libgxm */
	returnCode =initGxm();
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

    SceDbgFontConfig config;
	memset( &config, 0, sizeof(SceDbgFontConfig) );
	config.fontSize = SCE_DBGFONT_FONTSIZE_LARGE;

	returnCode = sceDbgFontInit( &config );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* Message for SDK sample auto test */
	printf( "## simple: INIT SUCCEEDED ##\n" );

	/* create gxm graphics data */
	createGxmData();

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALANALOG_WIDE);							//ButtonInput aktivieren
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);		//BackTouchInput aktivieren
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);		//FrontTouchInput aktivieren

	/* 6. main loop */
	while (true)
	{
        update();
		render();
		cycleDisplayBuffers();
	}

    
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
	
}

float makeFloat(unsigned char input){

	return (((float)(input) / 255.0f * 2.0f) -1.0f);
}

float fCurve(float x) {

	return ((-std::cos(x * std::acos(-1)) + 1) / 2);
}

int slicetype = 0; 
int sliceindex = 0;

float rotationProgress = 0.0f;
bool isAnimating = false;


int tmp_sliceindexH;
int tmp_sliceindexV;

SceTouchData frontTouchResult;
float frontTouch[2];
int savedFrontTouchId = -1;
bool frontTouchMoved;
bool axisIsDecided = false;

int touchedSide;

Vector2 z;
Vector2 h;
Vector2 k;
Vector2 rotationAxis;
bool invertRotationProgress;

float CubeSize = 0.9001f;

char debug_text[512];

//Nur in normalized device coordinates rechnen, dann ist es einfacher und konsistenter mit dem rechnen

static Plane planes[] = {
	//x-Ebenen
	Plane(Point3(CubeSize, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f)),		
	Plane(Point3(-CubeSize, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f)),

	//y-Ebenen
	Plane(Point3(0.0f, CubeSize, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),
	Plane(Point3(0.0f, -CubeSize, 0.0f), Vector3(0.0f, -1.0f, 0.0f)),

	//z-Ebenen
	Plane(Point3(0.0f, 0.0f, CubeSize), Vector3(0.0f, 0.0f, 1.0f)),
	Plane(Point3(0.0f, 0.0f, -CubeSize), Vector3(0.0f, 0.0f, -1.0f)),
};

//Achse um die gedreht werden soll, der sliceType und ob der RotationProgress invertiert werden muss
struct RotatingData {
    Vector4 axis;
    int sliceType;
    bool isInverted;
};

struct DecideData {
    RotatingData h;
    RotatingData k;
};

//Entscheidungsdaten um welche Achse gedreht werden soll
static DecideData decisionData[] = {
	{
		{ Vector4 (0.0f, 0.0f, 1.0f, 0.0f), 0, true },
		{ Vector4 (0.0f, 1.0f, 0.0f, 0.0f), 2, false }
	},
	{
		{ Vector4 (0.0f, 0.0f, 1.0f, 0.0f), 0, false },
		{ Vector4 (0.0f, 1.0f, 0.0f, 0.0f), 2, true }
	},
		{
		{ Vector4 (0.0f, 0.0f, 1.0f, 0.0f), 1, true },
		{ Vector4 (1.0f, 0.0f, 0.0f, 0.0f), 2, false }
	}, 
	{
		{ Vector4 (0.0f, 0.0f, 1.0f, 0.0f), 1, true },
		{ Vector4 (1.0f, 0.0f, 0.0f, 0.0f), 2, false }
	}, 
	{
		{ Vector4 (1.0f, 0.0f, 0.0f, 0.0f), 0, false },
		{ Vector4 (0.0f, 1.0f, 0.0f, 0.0f), 1, true }
	},
	{
		{ Vector4 (1.0f, 0.0f, 0.0f, 0.0f), 0, true },
		{ Vector4 (0.0f, 1.0f, 0.0f, 0.0f), 1, false }
	}
};


void checkFrontTouch(){
	SceTouchData frontTouchResult;
	sceTouchRead(SCE_TOUCH_PORT_FRONT, &frontTouchResult, 1);

	if(frontTouchResult.reportNum > 0){
		//Wenn eine ID mit Touchpunkt gemerkt wurde
		if(savedFrontTouchId != -1){

			//Wird gesetzt wenn sich der Finger bewegt
			frontTouchMoved = false;

			for(int i = 0; i < frontTouchResult.reportNum; ++i){
				if(savedFrontTouchId == frontTouchResult.report[i].id){
					//Vektor der Fingerbewegung
					z = Vector2(frontTouchResult.report[i].x, - frontTouchResult.report[i].y) - Vector2(frontTouch[0], - frontTouch[1]);

					if(axisIsDecided){
						if(!isAnimating)
							setAnimationFlag(s_basicVertices, slicetype, sliceindex);
						isAnimating = true;
						//Hier den Rotationprogress berechnen
						rotationProgress = dot(rotationAxis, z) * 0.001;

						if(invertRotationProgress){
							rotationProgress *= -1;
						}

						//Wert clippen
						if(rotationProgress > 1)
							rotationProgress = 1;
						if(rotationProgress < -1)
							rotationProgress = -1;
						

					}else{

						//Umrechnung nur um schönere Zahlen zu haben
						float len  = length(z) * 0.5f;

						if(len > 10){
							
							//decide Turning Axis
							axisIsDecided = true;

							if(!isAnimating){
								//Normalisieren, um auch bei verdrehtem Würfel um die richtige Achse zu drehen

								DecideData d = decisionData[touchedSide];

								h = normalize((s_finalTransformation * d.h.axis).getXY());
								k = normalize((s_finalTransformation * d.k.axis).getXY());
								
								if(abs(dot(h, z)) > abs(dot(k, z))){
									rotationAxis = h;
									slicetype = d.h.sliceType;
									sliceindex = tmp_sliceindexV;
									invertRotationProgress = d.h.isInverted;
								}else{
									rotationAxis = k;
									slicetype = d.k.sliceType;
									sliceindex = tmp_sliceindexH;
									invertRotationProgress = d.k.isInverted;
								}
							}
						}
					}
					frontTouchMoved = true;									
					break;
				}
			}
			//Wenn es keine Berührung mehr mit der gleichen ID gibt
			if(!frontTouchMoved){									
				savedFrontTouchId = -1;	
				axisIsDecided = false;
			}
			
		}else{

			//Ray mit neuem Touch erzeugen

			//Transformationsmatrix invertieren
			Matrix4 transformInverse = inverse(s_finalTransformation);

			//Startwerte der Berührung speichern
			frontTouch[0] =  2.0f * frontTouchResult.report[0].x / 1919.0f - 1.0f;
			frontTouch[1] = -2.0f * frontTouchResult.report[0].y / 1087.0f + 1.0f;

			//p1 - farPlane
			Vector4 p1 = transformInverse * Vector4(frontTouch[0], frontTouch[1], 0.9f, 1.0f);
			//p2 - nearPlane
			Vector4 p2 = transformInverse * Vector4(frontTouch[0], frontTouch[1], 0.1f, 1.0f);

			//Dehomogenisierungsdivision
			p1 = p1 / p1.getW();
			p2 = p2 / p2.getW();

			//Ray erstellen

			Point3 a = Point3(p1.getXYZ());
			Point3 b = Point3(p2.getXYZ());

			Line ray = Line(a, b);
			Vector3 d = ray.getDirection();
			Vector3 n;

			for(int i = 0; i < 6;++i){
				
				Plane tmp = planes[i];
				n = tmp.getNormal();

				if(dot(d, n) < 0){
				//Schnittpunkt mit Plane 
					Point3 intersection;
					if(!intersectionPoint(ray, tmp, &intersection)) continue;

					float x = intersection.getX();
					float y = intersection.getY();
					float z = intersection.getZ();

					if(x <= CubeSize && x >= -CubeSize					
						&& y <= CubeSize && y >= -CubeSize
						&& z <= CubeSize && z >= -CubeSize){

						//Sliceindex merken in 2 verschiedenen Variablen für die 2 Dimensionen
						if(i < 2){
							tmp_sliceindexH =     (int) (( z + CubeSize) / (CubeSize * 2) * 3);
							tmp_sliceindexV = 2 - (int) (( y + CubeSize) / (CubeSize * 2) * 3);
						} else if(i < 4){
							tmp_sliceindexH =     (int) (( z + CubeSize) / (CubeSize * 2) * 3);
							tmp_sliceindexV = 2 - (int) (( x + CubeSize) / (CubeSize * 2) * 3);
						} else if(i < 6){
							tmp_sliceindexH = 2 - (int) (( x + CubeSize) / (CubeSize * 2) * 3);
							tmp_sliceindexV = 2 - (int) (( y + CubeSize) / (CubeSize * 2) * 3);
						} else{
							printf("FATAL ERROR");
						}
						touchedSide = i;
						savedFrontTouchId = frontTouchResult.report[0].id;
						frontTouch[0] = frontTouchResult.report[0].x;		
						frontTouch[1] = frontTouchResult.report[0].y;
					}
				}
			}
		}
	}else{
		//Wenn kein Touch mehr vorhanden
		savedFrontTouchId = -1;
		axisIsDecided = false;
		
		float rotationSwitch = 0.4f;


		//bei angefangener Animation auch zu Ende animieren
		if(rotationProgress > rotationSwitch){
			rotationProgress += 0.05f;
			//printf("rotationProgressLoop\n");
		}else if (rotationProgress <= rotationSwitch && rotationProgress > 0){
			rotationProgress -= 0.05f;
			if(rotationProgress <= 0.05f){
				resetAnimationFlag(s_basicVertices);
				isAnimating = false;
				rotationProgress = 0.0f;
			}
		}


		if(rotationProgress < -rotationSwitch){
			rotationProgress -= 0.05f;
			//printf("rotationProgressLoop\n");
		}else if (rotationProgress >= -rotationSwitch && rotationProgress < 0){
			rotationProgress += 0.05f;
			if(rotationProgress >= -0.05f){
				resetAnimationFlag(s_basicVertices);
				isAnimating = false;
				rotationProgress = 0.0f;
			}
		}


		//do at end of Rotation
		if(rotationProgress >= 1.0f){
		
			cube->cube_turn_slice(slicetype, sliceindex, rotationProgress);
			resetAnimationFlag(s_basicVertices);	
			colorizeVertices(s_basicVertices);
			isAnimating = false;
			rotationProgress = 0.0f;
		}
		if(rotationProgress <= -1.0f){
			
			cube->cube_turn_slice(slicetype, sliceindex, rotationProgress);
			resetAnimationFlag(s_basicVertices);	
			colorizeVertices(s_basicVertices);
			isAnimating = false;
			rotationProgress = 0.0f;
		}	
	touchedSide = -1;
	}
}


float backX = 0.f;
float backY = 0.f;
SceTouchData backTouchResult;
float backTouch[2];
int savedBackTouchId = -1;
bool backTouchMoved;


void checkBackTouch(){

	// v v v BackTouch 
	sceTouchRead(SCE_TOUCH_PORT_BACK, &backTouchResult, 1);
	//getBackTouchInput

	//für anhaltend rotierenden Würfel, die nächsten beiden Zeilen auskommentieren
	backX = 0.0f;
	backY = 0.0f;

	if(backTouchResult.reportNum > 0){										//Wenn es min. eine TouchEingabe gibt
		if(savedBackTouchId != -1){												//wenn es noch eine TouchEingabe gibt
			backTouchMoved = false;
			for(int i = 0; i < backTouchResult.reportNum; ++i){			
				if(savedBackTouchId == backTouchResult.report[i].id){			//Solange der Touch mit der gleichen ID anhält
						
					backX = backTouchResult.report[i].x - backTouch[0];		//Bewegung entlang X-Achse 
					backY = backTouchResult.report[i].y - backTouch[1];		//Bewegung entlang Y-Achse 

					backTouch[0] = backTouchResult.report[i].x;				//Aufhören wenn der Finger stehen bleibt
					backTouch[1] = backTouchResult.report[i].y;				//Aufhören wenn der Finger stehen bleibt

					backTouchMoved = true;									
					break;
				}
			}
			if(!backTouchMoved){											//wenn keine Berührung mehr da ist wird die Id auf -1 gesetzt
				savedBackTouchId = -1;												
			}
		}else{																//wenn es eine neue TouchId gibt wird diese gespeichert inkl. Startpunkt
			savedBackTouchId = backTouchResult.report[0].id;
			backTouch[0] = backTouchResult.report[0].x;
			backTouch[1] = backTouchResult.report[0].y;
		}
	}
}



bool isRotating = false;
bool rotateLeft = false;
bool rotateRight = false;

void doButtonAnimation(){		
	setAnimationFlag(s_basicVertices, slicetype, sliceindex);

	if(rotateRight){
		rotationProgress += 0.05f;
		if(rotationProgress >= 1.0f){
			isRotating = false;
			rotateRight = false;
			rotationProgress = 0.0f;
			resetAnimationFlag(s_basicVertices);
			colorizeVertices(s_basicVertices);
		}
	}

	else if(rotateLeft){
		rotationProgress -= 0.05f;
		if(rotationProgress <= -1.0f){
			isRotating = false;
			rotateLeft = false;
			rotationProgress = 0.0f;
			resetAnimationFlag(s_basicVertices);
			colorizeVertices(s_basicVertices);
		}
	}
}

void checkButtonInput(){

	DebugUpdate(input, cube, slicetype, sliceindex, isRotating, rotateLeft, rotateRight);
}

static Quat m_orientationQuaternion(1.f, 0.f, 0.f, 0.f);


void calculateFinalTransformation(){
	Quat rotationVelocity(-backY * 0.002f, -backX * 0.002f, 0.f, 0.f);
	m_orientationQuaternion += 0.5f * rotationVelocity * m_orientationQuaternion;
	m_orientationQuaternion = normalize(m_orientationQuaternion);
	s_finalRotation = Matrix4(m_orientationQuaternion, Vector3(0.f, 0.f, 0.f));
	Matrix4 lookAt = Matrix4::lookAt(Point3(0.0f, 0.0f, 5.0f), Point3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));		//Die Kamera (Standpunkt, wohin man schaut, wo oben ist)
	Matrix4 perspective = Matrix4::perspective(3.141592f / 4.0f,															//fovyRadians				
	((float) DISPLAY_WIDTH/(float)DISPLAY_HEIGHT),																			//aspectRatio
	0.1f,																													//zNear
	10.0f);																													//zFar
	s_finalTransformation = perspective * lookAt * s_finalRotation;		
	
}

void update (void) {
	
	if(isRotating){

		 doButtonAnimation();

	} else {
		
		checkFrontTouch();

		checkBackTouch();

		//checkButtonInput();	
	
		calculateFinalTransformation();
	}	
}


/* Initialize libgxm */
int initGxm( void )
{
/* ---------------------------------------------------------------------
	2. Initialize libgxm

	First we must initialize the libgxm library by calling sceGxmInitialize.
	The single argument to this function is the size of the parameter buffer to
	allocate for the GPU.  We will use the default 16MiB here.

	Once initialized, we need to create a rendering context to allow to us
	to render scenes on the GPU.  We use the default initialization
	parameters here to set the sizes of the various context ring buffers.

	Finally we create a render target to describe the geometry of the back
	buffers we will render to.  This object is used purely to schedule
	rendering jobs for the given dimensions, the color surface and
	depth/stencil surface must be allocated separately.
	--------------------------------------------------------------------- */

	int returnCode = SCE_OK;

	/* set up parameters */
	SceGxmInitializeParams initializeParams;
	memset( &initializeParams, 0, sizeof(SceGxmInitializeParams) );						//alles mit 0/NULL initialisieren
	initializeParams.flags = 0;
	initializeParams.displayQueueMaxPendingCount = DISPLAY_MAX_PENDING_SWAPS;			//wie viel swaps kann ich speichern?
	initializeParams.displayQueueCallback = displayCallback;
	initializeParams.displayQueueCallbackDataSize = sizeof(DisplayData);
	initializeParams.parameterBufferSize = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;

	/* start libgxm */
	returnCode = sceGxmInitialize( &initializeParams );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* allocate ring buffer memory using default sizes */
	//Funktion kann übernommen werden, muss aber erklärt werden, wir wollen einen ungecashten speicher haben, weil die gpu mit gecashtem speicher auf probleme stoßen kann
	void *vdmRingBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE, 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_vdmRingBufferUId );
	//nur lesend drauf zugreifen, 
	void *vertexRingBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE, 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_vertexRingBufferUId );

	void *fragmentRingBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE, 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_fragmentRingBufferUId );

	uint32_t fragmentUsseRingBufferOffset;
	void *fragmentUsseRingBuffer = fragmentUsseAlloc( SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE, &s_fragmentUsseRingBufferUId, &fragmentUsseRingBufferOffset );

	/* create a rendering context */
	memset( &s_contextParams, 0, sizeof(SceGxmContextParams) );

	//normal mit malloc holen
	s_contextParams.hostMem = malloc( SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE );
	s_contextParams.hostMemSize = SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE;
	s_contextParams.vdmRingBufferMem = vdmRingBuffer;
	s_contextParams.vdmRingBufferMemSize = SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE;
	s_contextParams.vertexRingBufferMem = vertexRingBuffer;
	s_contextParams.vertexRingBufferMemSize = SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE;
	s_contextParams.fragmentRingBufferMem = fragmentRingBuffer;
	s_contextParams.fragmentRingBufferMemSize = SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE;
	s_contextParams.fragmentUsseRingBufferMem = fragmentUsseRingBuffer;
	s_contextParams.fragmentUsseRingBufferMemSize = SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE;
	s_contextParams.fragmentUsseRingBufferOffset = fragmentUsseRingBufferOffset;
	returnCode = sceGxmCreateContext( &s_contextParams, &s_context );								//hier wird der render context erzeugt
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* set buffer sizes for this sample */
	const uint32_t patcherBufferSize = 64*1024;
	const uint32_t patcherVertexUsseSize = 64*1024;
	const uint32_t patcherFragmentUsseSize = 64*1024;

	/* allocate memory for buffers and USSE code */
	void *patcherBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, patcherBufferSize, 4, SCE_GXM_MEMORY_ATTRIB_WRITE|SCE_GXM_MEMORY_ATTRIB_WRITE, &s_patcherBufferUId );

	uint32_t patcherVertexUsseOffset;
	void *patcherVertexUsse = vertexUsseAlloc( patcherVertexUsseSize, &s_patcherVertexUsseUId, &patcherVertexUsseOffset );

	uint32_t patcherFragmentUsseOffset;
	void *patcherFragmentUsse = fragmentUsseAlloc( patcherFragmentUsseSize, &s_patcherFragmentUsseUId, &patcherFragmentUsseOffset );

	/* create a shader patcher */
	SceGxmShaderPatcherParams patcherParams;
	memset( &patcherParams, 0, sizeof(SceGxmShaderPatcherParams) );
	patcherParams.userData = NULL;
	patcherParams.hostAllocCallback = &patcherHostAlloc;
	patcherParams.hostFreeCallback = &patcherHostFree;
	patcherParams.bufferAllocCallback = NULL;
	patcherParams.bufferFreeCallback = NULL;
	patcherParams.bufferMem = patcherBuffer;
	patcherParams.bufferMemSize = patcherBufferSize;
	patcherParams.vertexUsseAllocCallback = NULL;
	patcherParams.vertexUsseFreeCallback = NULL;
	patcherParams.vertexUsseMem = patcherVertexUsse;
	patcherParams.vertexUsseMemSize = patcherVertexUsseSize;
	patcherParams.vertexUsseOffset = patcherVertexUsseOffset;
	patcherParams.fragmentUsseAllocCallback = NULL;
	patcherParams.fragmentUsseFreeCallback = NULL;
	patcherParams.fragmentUsseMem = patcherFragmentUsse;
	patcherParams.fragmentUsseMemSize = patcherFragmentUsseSize;
	patcherParams.fragmentUsseOffset = patcherFragmentUsseOffset;
	returnCode = sceGxmShaderPatcherCreate( &patcherParams, &s_shaderPatcher );						//shader patcher erzeugen
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* create a render target */
	memset( &s_renderTargetParams, 0, sizeof(SceGxmRenderTargetParams) );
	s_renderTargetParams.flags = 0;
	s_renderTargetParams.width = DISPLAY_WIDTH;
	s_renderTargetParams.height = DISPLAY_HEIGHT;
	s_renderTargetParams.scenesPerFrame = 1;														//nur eine szene pro frame zeichnen
	s_renderTargetParams.multisampleMode = SCE_GXM_MULTISAMPLE_NONE;
	s_renderTargetParams.multisampleLocations	= 0;
	s_renderTargetParams.driverMemBlock = SCE_UID_INVALID_UID;

	returnCode = sceGxmCreateRenderTarget( &s_renderTargetParams, &s_renderTarget );				//render target erzeugen
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );


/* ---------------------------------------------------------------------
	3. Allocate display buffers, set up the display queue

	We will allocate our back buffers in CDRAM, and create a color
	surface for each of them.

	To allow display operations done by the CPU to be synchronized with
	rendering done by the GPU, we also create a SceGxmSyncObject for each
	display buffer.  This sync object will be used with each scene that
	renders to that buffer and when queueing display flips that involve
	that buffer (either flipping from or to).

	Finally we create a display queue object that points to our callback
	function.
	--------------------------------------------------------------------- */

	/* allocate memory and sync objects for display buffers */
	for ( unsigned int i = 0 ; i < DISPLAY_BUFFER_COUNT ; ++i )
	{
		/* allocate memory with large size to ensure physical contiguity */
		s_displayBufferData[i] = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA/*display buffer muss im cd ram liegen*/, ALIGN(4*DISPLAY_STRIDE_IN_PIXELS*DISPLAY_HEIGHT/*4 Byte pro Pixel werden benötigt*/, 1*1024*1024), SCE_GXM_COLOR_SURFACE_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ|SCE_GXM_MEMORY_ATTRIB_WRITE, &s_displayBufferUId[i] );
		SCE_DBG_ALWAYS_ASSERT( s_displayBufferData[i] );

		/* memset the buffer to debug color */
		//alle pixel schwarz färben
		for ( unsigned int y = 0 ; y < DISPLAY_HEIGHT ; ++y )
		{
			unsigned int *row = (unsigned int *)s_displayBufferData[i] + y*DISPLAY_STRIDE_IN_PIXELS;

			for ( unsigned int x = 0 ; x < DISPLAY_WIDTH ; ++x )
			{
				row[x] = 0x0;					//von hand eingetragen, von der cpu alles mit schwarz auffüllen
			}
		}

		/* initialize a color surface for this display buffer */
		returnCode = sceGxmColorSurfaceInit( &s_displaySurface[i], DISPLAY_COLOR_FORMAT, SCE_GXM_COLOR_SURFACE_LINEAR, SCE_GXM_COLOR_SURFACE_SCALE_NONE,
											 SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_STRIDE_IN_PIXELS, s_displayBufferData[i] );
		SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

		/* create a sync object that we will associate with this buffer */
		returnCode = sceGxmSyncObjectCreate( &s_displayBufferSync[i] );				//muss immer ein vilefaches von der Kachelgröße sein, Speichergröße für tiefenpuffer
		SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	}

	/* compute the memory footprint of the depth buffer */
	const uint32_t alignedWidth = ALIGN( DISPLAY_WIDTH, SCE_GXM_TILE_SIZEX );		//tiefenpuffer muss tile - weise hinterlegt sein
	const uint32_t alignedHeight = ALIGN( DISPLAY_HEIGHT, SCE_GXM_TILE_SIZEY );
	uint32_t sampleCount = alignedWidth*alignedHeight;
	uint32_t depthStrideInSamples = alignedWidth;

	/* allocate it */
	void *depthBufferData = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 4*sampleCount, SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ|SCE_GXM_MEMORY_ATTRIB_WRITE, &s_depthBufferUId );

	/* create the SceGxmDepthStencilSurface structure */
	returnCode = sceGxmDepthStencilSurfaceInit( &s_depthSurface, SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24, SCE_GXM_DEPTH_STENCIL_SURFACE_TILED, depthStrideInSamples, depthBufferData, NULL );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	return returnCode;
}

/*erstellt eine Kachel von einer Seite von dem Würfel*/
void CreateCubeSideTile(BasicVertex* field, int type, int direction, int x, int y){

	/*
	type 0 senkrecht zu x, 1 senkrecht zu y, 2 senkrecht zur z-Achse
	direction 0 oder 1, quasi die 2 Ausrichtungen bzw. gegenüberliegende Seiten
	x, y gibt an wie weit entfernt die Kachel gezeichnet werden soll
	*/

	//eigentlich nur die Hälfte der CubeSize
	const float CUBE_SIZE = .9f;
	const float TILE_SIZE_HALF = .3f;

	float debugGap = 0.01f;

	/*durchläuft die 4 Vertices der Kachel und setzt den Abstand der Vertices zueinander in Richtung der Übergebenen Achse*/
	for( int i = 0; i < 4; ++i){
		field[i].position[type] = direction * CUBE_SIZE;
		field[i].ANIMATION_FLAG = -1;
	}

	/*Setzt die Lokalen X und Y Achsen der Kachel*/
	int localXDim = (type + 1) %3;
	int localYDim = (type + 2) %3;

	/*setzt den Abstand der 4 Vertices entlang der lokalen X und Y Achse*/
	field[0].position[localXDim] = -TILE_SIZE_HALF + (2 * x * TILE_SIZE_HALF + debugGap);
	field[0].position[localYDim] =  TILE_SIZE_HALF + (2 * y * TILE_SIZE_HALF - debugGap);

	field[1].position[localXDim] = -TILE_SIZE_HALF + (2 * x * TILE_SIZE_HALF + debugGap);
	field[1].position[localYDim] = -TILE_SIZE_HALF + (2 * y * TILE_SIZE_HALF + debugGap);

	field[2].position[localXDim] =  TILE_SIZE_HALF + (2 * x * TILE_SIZE_HALF - debugGap);
	field[2].position[localYDim] = -TILE_SIZE_HALF + (2 * y * TILE_SIZE_HALF + debugGap);

	field[3].position[localXDim] = TILE_SIZE_HALF +  (2 * x * TILE_SIZE_HALF - debugGap);
	field[3].position[localYDim] = TILE_SIZE_HALF +  (2 * y * TILE_SIZE_HALF - debugGap);


	//Die normale der 4 Vertices berechnen, damit sie in die selbe Richtung orientiert sind
	//hat nur die komponente gesetzt zu welcher achse ich senkrecht bin, also entweder x, y oder z. ist ja in type gespeichert

	float normal[3];
	normal[0] = normal[1] = normal[2] = 0.0f;
	normal[type] = direction;

	//allen 4 Vertices die gleiche Normale geben
	for(int i = 0; i < 4; ++i){
		for(int j = 0; j < 3; ++j){
			field[i].normal[j] = normal[j];
		}
	}
}



void colorizeVertices(BasicVertex* vertices){

	int count = 0;

	for(int side=0; side < 6; ++side){
		for(int row = 0; row < 3; ++row){
			for(int column = 0; column < 3; ++column){

				uint32_t tmpColor = cube->getColor(side, row, column);

				vertices[count++].color = tmpColor;
				vertices[count++].color = tmpColor;
				vertices[count++].color = tmpColor;
				vertices[count++].color = tmpColor;
			}
		}
	}
}


void setAnimationFlagSide(BasicVertex* vertices, int side, int flag){

	for(int i = 0; i < 36; i++){
		vertices[side * 36 + i].ANIMATION_FLAG = flag;
	}
}


// HACK HACK HACK
bool g_isAnimationSet = false;
// HACK HACK HACK

void setAnimationFlag(BasicVertex* vertices, int dimension, int slice){

	/*
	assert(!g_isAnimationSet);
	g_isAnimationSet = true;
	*/
	if(dimension == 0){			//setzt das Animation Flag für alle Vertices in x-Richtung
		for(int vertex = 0; vertex < 12; vertex++){
			vertices[0 * 36 + slice * 3 * 4 + vertex].ANIMATION_FLAG = 0;
			vertices[1 * 36 + slice * 3 * 4 + vertex].ANIMATION_FLAG = 0;
			vertices[2 * 36 + slice * 3 * 4 + vertex].ANIMATION_FLAG = 0;
			vertices[3 * 36 + slice * 3 * 4 + vertex].ANIMATION_FLAG = 0;
		}	
		if(slice == 0)
			setAnimationFlagSide(vertices, 4, 0);

		else if(slice == 2)
			setAnimationFlagSide(vertices, 5, 0);

	}
	else if(dimension == 1){	//setzt das Animation Flag für alle Vertices in y-Richtung
		
		for(int tile = 0; tile < 3; tile++){
			for(int vertex = 0; vertex < 4; vertex++){

				vertices[0 * 36 + 4 * slice + tile * 12 + vertex].ANIMATION_FLAG = 1;
				vertices[4 * 36 + 4 * slice + tile * 12 + vertex].ANIMATION_FLAG = 1;
				vertices[5 * 36 + 4 * slice + tile * 12 + vertex].ANIMATION_FLAG = 1;
				vertices[2 * 36 + 4 * (2-slice) + tile * 12 + vertex].ANIMATION_FLAG = 1;
			}
		}
		if(slice == 0)
			setAnimationFlagSide(vertices, 3, 1);

		else if(slice == 2)
			setAnimationFlagSide(vertices, 1, 1);
	}
	else if(dimension == 2){	//setzt das Animation Flag für alle Vertices in z-Richtung

		for(int vertex = 0; vertex < 12; vertex++){
				vertices[4 * 36 + (2-slice) * 3 * 4 + vertex].ANIMATION_FLAG = 2;			//slice von unten nach oben
		}

		for(int tile = 0; tile < 3; tile++){
			for(int vertex = 0; vertex < 4; vertex++){
				vertices[3 * 36 + (2-slice) * 4 + tile * 12 + vertex].ANIMATION_FLAG = 2;	//slice rechts nach links
			}
		}

		for(int vertex = 0; vertex < 12; vertex++){
				vertices[5 * 36 + slice * 4 * 3 + vertex].ANIMATION_FLAG = 2;				//von oben nach unten
		}

		for(int tile = 0; tile < 3; tile++){
			for(int vertex = 0; vertex < 4; vertex++){
				vertices[1 * 36 + slice * 4 + tile * 12 + vertex].ANIMATION_FLAG = 2;		//von links nach rechts
			}
		}

		if(slice == 0)
			setAnimationFlagSide(vertices, 0, 2);

		else if(slice == 2)
			setAnimationFlagSide(vertices, 2, 2);
	}

	//printf("set AnimationFlags\n");
}

void resetAnimationFlag(BasicVertex* vertices){

	/*
	assert(g_isAnimationSet);
	g_isAnimationSet = false;
	*/

	for(int vertex = 0; vertex < (6 * 9 * 4); ++vertex){
		vertices[vertex].ANIMATION_FLAG = -1;
	}
	//printf("reset AnimationFlags\n");
}

void CreateTheCube(){

	//Create the Cube!______________________________________________________________________________________________________________________________________

	/*													0  1  2								________|_4_|___ 
	jede Seite wird nach folgendem Muster erzeugt:		3  4  5			Das Würfelnetz:		|_2_|_3_|_0_|_1_|
														6  7  8										| 5 |		*/

	int count = 0;
	for(int y = 1; y > -2; --y){
		for(int x = 1; x > -2; --x){
			CreateCubeSideTile(&(s_basicVertices[count]), 2, -1, x, y);
			count += 4;
		}
	}

	for(int x = 1; x > -2; --x){
		for(int y = -1; y < 2; ++y){
			CreateCubeSideTile(&(s_basicVertices[count]), 0, -1, x, y);
			count += 4;
		}
	}

	for(int y = 1; y > -2; --y){
		for(int x = -1; x < 2; ++x){
			CreateCubeSideTile(&(s_basicVertices[count]), 2, 1, x, y);
			count += 4;
		}
	}

	for(int x = 1; x > -2; --x){
		for(int y = 1; y > -2; --y){
			CreateCubeSideTile(&(s_basicVertices[count]), 0, 1, x, y);
			count += 4;
		}
	}

	for(int x = 1; x > -2; --x){
		for(int y = 1; y > -2; --y){
			CreateCubeSideTile(&(s_basicVertices[count]), 1, 1, x, y);
			count += 4;
		}
	}

	for(int x = -1; x < 2; ++x){
		for(int y = 1; y > -2; --y){
			CreateCubeSideTile(&(s_basicVertices[count]), 1, -1, x, y);
			count += 4;
		}
	}

	//__________________________________________________________________________________________________________________________
	count = 0;
	int baseIndex = 0;
	for(int side = 0; side < 6; ++side){

		baseIndex = side * 9 * 4;	//weil 9 tiles pro side a 4 vertices

		for(int tile = 0; tile < 9; ++tile){						

			s_basicIndices[count++] = baseIndex;
			s_basicIndices[count++] = baseIndex + 1;
			s_basicIndices[count++] = baseIndex + 2;

			s_basicIndices[count++] = baseIndex;
			s_basicIndices[count++] = baseIndex + 3;
			s_basicIndices[count++] = baseIndex + 2;

			baseIndex += 4;		//zu den nächsten 4 Indizes springen
		}
	}
	colorizeVertices(s_basicVertices);
}


/* Create libgxm scenes */
void createGxmData( void )
{
/* ---------------------------------------------------------------------
	4. Create a shader patcher and register programs

	A shader patcher object is required to produce vertex and fragment
	programs from the shader compiler output.  First we create a shader
	patcher instance, using callback functions to allow it to allocate
	and free host memory for internal state.

	In order to create vertex and fragment programs for a particular
	shader, the compiler output must first be registered to obtain an ID
	for that shader.  Within a single ID, vertex and fragment programs
	are reference counted and could be shared if created with identical
	parameters.  To maximise this sharing, programs should only be
	registered with the shader patcher once if possible, so we will do
	this now.
	--------------------------------------------------------------------- */

	/* register programs with the patcher */
	int returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryClearVGxpStart, &s_clearVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryClearFGxpStart, &s_clearFragmentProgramId );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );


    returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryBasicVGxpStart, &s_basicVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryBasicFGxpStart, &s_basicFragmentProgramId );
    SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );


/* ---------------------------------------------------------------------
	5. Create the programs and data for the clear

	On SGX hardware, vertex programs must perform the unpack operations
	on vertex data, so we must define our vertex formats in order to
	create the vertex program.  Similarly, fragment programs must be
	specialized based on how they output their pixels and MSAA mode
	(and texture format on ES1).

	We define the clear geometry vertex format here and create the vertex
	and fragment program.

	The clear vertex and index data is static, we allocate and write the
	data here.
	--------------------------------------------------------------------- */

	/* get attributes by name to create vertex format bindings */
	const SceGxmProgram *clearProgram = sceGxmShaderPatcherGetProgramFromId( s_clearVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( clearProgram );
	const SceGxmProgramParameter *paramClearPositionAttribute = sceGxmProgramFindParameterByName( clearProgram, "aPosition" );	//auf aPOSITION in .cg datei prüfen, clear_v
	SCE_DBG_ALWAYS_ASSERT( paramClearPositionAttribute && ( sceGxmProgramParameterGetCategory(paramClearPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	/* create clear vertex format */
	SceGxmVertexAttribute clearVertexAttributes[1];		//ein parameter im vertex shader
	SceGxmVertexStream clearVertexStreams[1];
	clearVertexAttributes[0].streamIndex = 0;
	clearVertexAttributes[0].offset = 0;
	clearVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;		//hier kann man entscheiden wie viel bit spendiert werden wollen
	clearVertexAttributes[0].componentCount = 2;
	clearVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex( paramClearPositionAttribute );
	clearVertexStreams[0].stride = sizeof(ClearVertex);
	clearVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	/* create clear programs */
	returnCode = sceGxmShaderPatcherCreateVertexProgram( s_shaderPatcher, s_clearVertexProgramId, clearVertexAttributes, 1, clearVertexStreams, 1, &s_clearVertexProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	returnCode = sceGxmShaderPatcherCreateFragmentProgram( s_shaderPatcher, s_clearFragmentProgramId,
														   SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4/*RGBA Information*/, SCE_GXM_MULTISAMPLE_NONE, NULL,
														   sceGxmShaderPatcherGetProgramFromId(s_clearVertexProgramId), &s_clearFragmentProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* create the clear triangle vertex/index data */
	s_clearVertices = (ClearVertex *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 3*sizeof(ClearVertex)/*für die 3 Ecken des Dreiecks*/, 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_clearVerticesUId );	//Speicher zum zeichnen hinterlegen, uncashed um von gpu gelesen werden zu können
	s_clearIndices = (uint16_t *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 3*sizeof(uint16_t), 2, SCE_GXM_MEMORY_ATTRIB_READ, &s_clearIndicesUId );

	
	//die 3 Ecken vom dreieck das den Bildschirm löscht zeichnen
	s_clearVertices[0].x = -1.0f;
	s_clearVertices[0].y = -1.0f;
	s_clearVertices[1].x =  3.0f;
	s_clearVertices[1].y = -1.0f;
	s_clearVertices[2].x = -1.0f;
	s_clearVertices[2].y =  3.0f;


	s_clearIndices[0] = 0;
	s_clearIndices[1] = 1;
	s_clearIndices[2] = 2;

    // !! All related to triangle.

	/*alles nochmal für das farbige Dreieck das danach gezeichnet wird*/

    /* get attributes by name to create vertex format bindings */
	/* first retrieve the underlying program to extract binding information */
	const SceGxmProgram *basicProgram = sceGxmShaderPatcherGetProgramFromId( s_basicVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( basicProgram );


	/*__________________________________________________________________________________________________________________________*/

	//Cube zeichnen
	const SceGxmProgramParameter *paramBasicPositionAttribute = sceGxmProgramFindParameterByName( basicProgram, "aPosition" );											//name der Position aus dem VertexShader
	SCE_DBG_ALWAYS_ASSERT( paramBasicPositionAttribute && ( sceGxmProgramParameterGetCategory(paramBasicPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	const SceGxmProgramParameter *paramBasicNormalAttribute = sceGxmProgramFindParameterByName(basicProgram, "aNormal");												//Normale abfragen
	SCE_DBG_ALWAYS_ASSERT( paramBasicNormalAttribute && ( sceGxmProgramParameterGetCategory(paramBasicNormalAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );


	const SceGxmProgramParameter *paramBasicColorAttribute = sceGxmProgramFindParameterByName( basicProgram, "aColor" );												//name der Farbe aus dem Shader
	SCE_DBG_ALWAYS_ASSERT( paramBasicColorAttribute && ( sceGxmProgramParameterGetCategory(paramBasicColorAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	const SceGxmProgramParameter *paramBasicFlagAttribute = sceGxmProgramFindParameterByName( basicProgram, "ANIMATION_FLAG" );
	SCE_DBG_ALWAYS_ASSERT( paramBasicFlagAttribute && ( sceGxmProgramParameterGetCategory(paramBasicFlagAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );
	

	/* create shaded triangle vertex format */
	SceGxmVertexAttribute basicVertexAttributes[4];
	SceGxmVertexStream basicVertexStreams[1];

	//Komponente 0 ist Position
	basicVertexAttributes[0].streamIndex = 0;
	basicVertexAttributes[0].offset = 0;
	basicVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;		//ist 32Bit, 4 Byte dick
	basicVertexAttributes[0].componentCount = 3;
	basicVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicPositionAttribute );


	//Komponente 1 ist Normale(für Beleuchtung)
	basicVertexAttributes[1].streamIndex = 0;
	basicVertexAttributes[1].offset = 12;								//weil 3*4 Bytes für die position vor der farbe
	basicVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;		// Mapping relation clarified. u8n = unsigned8normalized (0-255 abbilden auf fließkommawerte 0-1)
	basicVertexAttributes[1].componentCount = 3; 
	basicVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicNormalAttribute );

	//Komponente 2 ist Farbe
	basicVertexAttributes[2].streamIndex = 0;
	basicVertexAttributes[2].offset = 24;		//3 für position, 3 für normale, dann farbe
	basicVertexAttributes[2].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N; // Mapping relation clarified. u8n = unsigned8normalized (0-255 abbilden auf fließkommawerte 0-1)
	basicVertexAttributes[2].componentCount = 4;
	basicVertexAttributes[2].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicColorAttribute );

	//Komponente 3 für ANIMATION_FLAG
	basicVertexAttributes[3].streamIndex = 0;
	basicVertexAttributes[3].offset = 28;		
	basicVertexAttributes[3].format = SCE_GXM_ATTRIBUTE_FORMAT_S8;
	basicVertexAttributes[3].componentCount = 1;
	basicVertexAttributes[3].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicFlagAttribute );


	basicVertexStreams[0].stride = sizeof(BasicVertex);
	basicVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	/* create shaded triangle shaders */
	//Programme anmelden
	returnCode = sceGxmShaderPatcherCreateVertexProgram( s_shaderPatcher, s_basicVertexProgramId, basicVertexAttributes, 4,
														 basicVertexStreams, 1, &s_basicVertexProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	returnCode = sceGxmShaderPatcherCreateFragmentProgram( s_shaderPatcher, s_basicFragmentProgramId,
														   SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4, SCE_GXM_MULTISAMPLE_NONE, NULL,
														   sceGxmShaderPatcherGetProgramFromId(s_basicVertexProgramId), &s_basicFragmentProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* find vertex uniforms by name and cache parameter information */
	s_wvpParam = sceGxmProgramFindParameterByName( basicProgram, "wvp" );  //im vertexshader wvp abfragen
	SCE_DBG_ALWAYS_ASSERT( s_wvpParam && ( sceGxmProgramParameterGetCategory( s_wvpParam ) == SCE_GXM_PARAMETER_CATEGORY_UNIFORM /*gucken ob der auch vom wert uniform ist*/));

	s_rotParam = sceGxmProgramFindParameterByName( basicProgram, "rot" );  //im vertexshader rot abfragen
	SCE_DBG_ALWAYS_ASSERT( s_rotParam && ( sceGxmProgramParameterGetCategory( s_rotParam ) == SCE_GXM_PARAMETER_CATEGORY_UNIFORM));

	s_rotAngleParam = sceGxmProgramFindParameterByName( basicProgram, "angle" );
	SCE_DBG_ALWAYS_ASSERT( s_rotAngleParam && ( sceGxmProgramParameterGetCategory( s_rotAngleParam ) == SCE_GXM_PARAMETER_CATEGORY_UNIFORM));

	/* create shaded triangle vertex/index data */
	s_basicVertices = (BasicVertex *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, /*hier dann 9* einfügen um die 9 kacheln pro seite zeichnen zu können*/9*4*6*sizeof(BasicVertex), 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_basicVerticesUId );
	s_basicIndices = (uint16_t *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 9*6*6*sizeof(uint16_t), 2, SCE_GXM_MEMORY_ATTRIB_READ, &s_basicIndiceUId );


	CreateTheCube();
}

/* Main render function */
void render( void )
{
	/* render libgxm scenes */
	renderGxm();
}

/* render gxm scenes */
void renderGxm( void )
{
/* -----------------------------------------------------------------
	8. Rendering step

	This sample renders a single scene containing the clear triangle.
	Before any drawing can take place, a scene must be started.
	We render to the back buffer, so it is also important to use a
	sync object to ensure that these rendering operations are
	synchronized with display operations.

	The clear triangle shaders do not declare any uniform variables,
	so this may be rendered immediately after setting the vertex and
	fragment program.

	Once clear triangle have been drawn the scene can be ended, which
	submits it for rendering on the GPU.
	----------------------------------------------------------------- */

	/* start rendering to the render target */
	sceGxmBeginScene( s_context, 0, s_renderTarget, NULL, NULL, s_displayBufferSync[s_displayBackBufferIndex], &s_displaySurface[s_displayBackBufferIndex], &s_depthSurface );

	/* set clear shaders */
	sceGxmSetVertexProgram( s_context, s_clearVertexProgram );
	sceGxmSetFragmentProgram( s_context, s_clearFragmentProgram );

	/* draw ther clear triangle */
	sceGxmSetVertexStream( s_context, 0, s_clearVertices );
	sceGxmDraw( s_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, s_clearIndices, 3 );


    // !! Speciality for rendering a triangle.

    /* render the  triangle */
	sceGxmSetVertexProgram( s_context, s_basicVertexProgram );
	sceGxmSetFragmentProgram( s_context, s_basicFragmentProgram );

	/* set the vertex program constants */
	void *vertexDefaultBuffer;							//informationen wo die uniform data hinterlegt ist(im moment noch im hauptspeicher)

	sceGxmReserveVertexDefaultUniformBuffer( s_context, &vertexDefaultBuffer );

	sceGxmSetUniformDataF( vertexDefaultBuffer, s_wvpParam, 0, 16, (float*)&s_finalTransformation	/*casten, weil Matrix 4 genau 16 floats beinhaltet, diese sind auch schon in der richtigen reihenfolge*/);
	sceGxmSetUniformDataF( vertexDefaultBuffer, s_rotParam, 0, 16, (float*)&s_finalRotation);		/*auch die zweite Matrix an den Vertex Shader übergeben*/

	float angle = ((rotationProgress < 0) ? -1 : 1) * fCurve(fCurve(rotationProgress)) * 3.1415 * 0.5f;
	sceGxmSetUniformDataF( vertexDefaultBuffer, s_rotAngleParam, 0, 1, (float*)&angle);

	/* draw the spinning triangle */
	sceGxmSetVertexStream( s_context, 0, s_basicVertices );

	//Würfel : 6 Seiten a 9 Kacheln a 2 Dreiecke a 3 Indices
	//Welche Vertices werden zu Dreiecken zusammengebaut

	//Würfel zeichnen
	sceGxmDraw( s_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, s_basicIndices, 6 * 9 * 2 * 3 ); //hier wird der Würfel gezeichnet - dafür müssen alle 9 * 6 * 6 Indizes der zu zeichnenden Dreiecke durchlaufen werden

	/* stop rendering to the render target */
	sceGxmEndScene( s_context, NULL, NULL );
}



/* queue a display swap and cycle our buffers */
void cycleDisplayBuffers( void )
{
/* -----------------------------------------------------------------
	9-a. Flip operation

	Now we have finished submitting rendering work for this frame it
	is time to submit a flip operation.  As part of specifying this
	flip operation we must provide the sync objects for both the old
	buffer and the new buffer.  This is to allow synchronization both
	ways: to not flip until rendering is complete, but also to ensure
	that future rendering to these buffers does not start until the
	flip operation is complete.

	Once we have queued our flip, we manually cycle through our back
	buffers before starting the next frame.
	----------------------------------------------------------------- */

	/* PA heartbeat to notify end of frame */
	sceGxmPadHeartbeat( &s_displaySurface[s_displayBackBufferIndex], s_displayBufferSync[s_displayBackBufferIndex] );

	/* queue the display swap for this frame */
	DisplayData displayData;
	displayData.address = s_displayBufferData[s_displayBackBufferIndex];

	/* front buffer is OLD buffer, back buffer is NEW buffer */
	sceGxmDisplayQueueAddEntry( s_displayBufferSync[s_displayFrontBufferIndex], s_displayBufferSync[s_displayBackBufferIndex], &displayData );

	/* update buffer indices */
	s_displayFrontBufferIndex = s_displayBackBufferIndex;
	s_displayBackBufferIndex = (s_displayBackBufferIndex + 1) % DISPLAY_BUFFER_COUNT;
}


/* Destroy Gxm Data */
void destroyGxmData( void )
{
/* ---------------------------------------------------------------------
	11. Destroy the programs and data for the clear and spinning triangle

	Once the GPU is finished, we release all our programs.
	--------------------------------------------------------------------- */

	/* clean up allocations */
	sceGxmShaderPatcherReleaseFragmentProgram( s_shaderPatcher, s_clearFragmentProgram );
	sceGxmShaderPatcherReleaseVertexProgram( s_shaderPatcher, s_clearVertexProgram );
	graphicsFree( s_clearIndicesUId );
	graphicsFree( s_clearVerticesUId );

	/* wait until display queue is finished before deallocating display buffers */
	sceGxmDisplayQueueFinish();

	/* unregister programs and destroy shader patcher */
	sceGxmShaderPatcherUnregisterProgram( s_shaderPatcher, s_clearFragmentProgramId );
	sceGxmShaderPatcherUnregisterProgram( s_shaderPatcher, s_clearVertexProgramId );
	sceGxmShaderPatcherDestroy( s_shaderPatcher );
	fragmentUsseFree( s_patcherFragmentUsseUId );
	vertexUsseFree( s_patcherVertexUsseUId );
	graphicsFree( s_patcherBufferUId );
}



/* ShutDown libgxm */
int shutdownGxm( void )
{
/* ---------------------------------------------------------------------
	12. Finalize libgxm

	Once the GPU is finished, we deallocate all our memory,
	destroy all object and finally terminate libgxm.
	--------------------------------------------------------------------- */

	int returnCode = SCE_OK;

	graphicsFree( s_depthBufferUId );

	for ( unsigned int i = 0 ; i < DISPLAY_BUFFER_COUNT; ++i )
	{
		memset( s_displayBufferData[i], 0, DISPLAY_HEIGHT*DISPLAY_STRIDE_IN_PIXELS*4 );
		graphicsFree( s_displayBufferUId[i] );
		sceGxmSyncObjectDestroy( s_displayBufferSync[i] );
	}

	/* destroy the render target */
	sceGxmDestroyRenderTarget( s_renderTarget );

	/* destroy the context */
	sceGxmDestroyContext( s_context );

	fragmentUsseFree( s_fragmentUsseRingBufferUId );
	graphicsFree( s_fragmentRingBufferUId );
	graphicsFree( s_vertexRingBufferUId );
	graphicsFree( s_vdmRingBufferUId );
	free( s_contextParams.hostMem );

	/* terminate libgxm */
	sceGxmTerminate();

	return returnCode;
}


/* Host alloc */
static void *patcherHostAlloc( void *userData, unsigned int size )
{
	(void)( userData );
	return malloc( size );
}

/* Host free */
static void patcherHostFree( void *userData, void *mem )
{
	(void)( userData );
	free( mem );
}

/* Display callback */
void displayCallback( const void *callbackData )
{
/* -----------------------------------------------------------------
	10-b. Flip operation

	The callback function will be called from an internal thread once
	queued GPU operations involving the sync objects is complete.
	Assuming we have not reached our maximum number of queued frames,
	this function returns immediately.
	----------------------------------------------------------------- */

	SceDisplayFrameBuf framebuf;

	/* cast the parameters back */
	const DisplayData *displayData = (const DisplayData *)callbackData;

    // Render debug text.
    /* set framebuffer info */
	SceDbgFontFrameBufInfo info;
	memset( &info, 0, sizeof(SceDbgFontFrameBufInfo) );
	info.frameBufAddr = (SceUChar8 *)displayData->address;
	info.frameBufPitch = DISPLAY_STRIDE_IN_PIXELS;
	info.frameBufWidth = DISPLAY_WIDTH;
	info.frameBufHeight = DISPLAY_HEIGHT;
	info.frameBufPixelformat = DBGFONT_PIXEL_FORMAT;

	/* flush font buffer */
	int returnCode = sceDbgFontFlush( &info );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	
	/* wwap to the new buffer on the next VSYNC */
	memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
	framebuf.size        = sizeof(SceDisplayFrameBuf);
	framebuf.base        = displayData->address;
	framebuf.pitch       = DISPLAY_STRIDE_IN_PIXELS;
	framebuf.pixelformat = DISPLAY_PIXEL_FORMAT;
	framebuf.width       = DISPLAY_WIDTH;
	framebuf.height      = DISPLAY_HEIGHT;
	returnCode = sceDisplaySetFrameBuf( &framebuf, SCE_DISPLAY_UPDATETIMING_NEXTVSYNC );		//erst beim nächsten vsync anzeigen
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* block this callback until the swap has occurred and the old buffer is no longer displayed */
	returnCode = sceDisplayWaitVblankStart();													//hier warten
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
}

/* Alloc used by libgxm */
static void *graphicsAlloc( SceKernelMemBlockType type, uint32_t size, uint32_t alignment, uint32_t attribs, SceUID *uid )
{
/*	Since we are using sceKernelAllocMemBlock directly, we cannot directly
	use the alignment parameter.  Instead, we must allocate the size to the
	minimum for this memblock type, and just SCE_DBG_ALWAYS_ASSERT that this will cover
	our desired alignment.

	Developers using their own heaps should be able to use the alignment
	parameter directly for more minimal padding.
*/

	if( type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA )
	{
		/* CDRAM memblocks must be 256KiB aligned */
		SCE_DBG_ALWAYS_ASSERT( alignment <= 256*1024 );
		size = ALIGN( size, 256*1024 );
	}
	else
	{
		/* LPDDR memblocks must be 4KiB aligned */
		SCE_DBG_ALWAYS_ASSERT( alignment <= 4*1024 );
		size = ALIGN( size, 4*1024 );
	}

	/* allocate some memory */
	*uid = sceKernelAllocMemBlock( "simple", type, size, NULL );
	SCE_DBG_ALWAYS_ASSERT( *uid >= SCE_OK );

	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( *uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* map for the GPU */
	//mappen, damit die gpu lesend/schreibend zugreifen kann
	err = sceGxmMapMemory( mem, size, attribs );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* done */
	return mem;
}

/* Free used by libgxm */
static void graphicsFree( SceUID uid )
{
	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase(uid, &mem);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	// unmap memory
	err = sceGxmUnmapMemory(mem);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	// free the memory block
	err = sceKernelFreeMemBlock(uid);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);
}

/* vertex alloc used by libgxm */
static void *vertexUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset )
{
	/* align to memblock alignment for LPDDR */
	size = ALIGN( size, 4096 );

	/* allocate some memory */
	*uid = sceKernelAllocMemBlock( "simple", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL );
	SCE_DBG_ALWAYS_ASSERT( *uid >= SCE_OK );

	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( *uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* map as vertex USSE code for the GPU */
	err = sceGxmMapVertexUsseMemory( mem, size, usseOffset );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	return mem;
}

/* vertex free used by libgxm */
static void vertexUsseFree( SceUID uid )
{
	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( uid, &mem );
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	/* unmap memory */
	err = sceGxmUnmapVertexUsseMemory( mem );
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	/* free the memory block */
	err = sceKernelFreeMemBlock( uid );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );
}

/* fragment alloc used by libgxm */
static void *fragmentUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset )
{
	/* align to memblock alignment for LPDDR */
	size = ALIGN( size, 4096 );

	/* allocate some memory */
	*uid = sceKernelAllocMemBlock( "simple", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL );
	SCE_DBG_ALWAYS_ASSERT( *uid >= SCE_OK );

	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( *uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* map as fragment USSE code for the GPU */
	err = sceGxmMapFragmentUsseMemory( mem, size, usseOffset);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	// done
	return mem;
}

/* fragment free used by libgxm */
static void fragmentUsseFree( SceUID uid )
{
	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* unmap memory */
	err = sceGxmUnmapFragmentUsseMemory( mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* free the memory block */
	err = sceKernelFreeMemBlock( uid );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );
}