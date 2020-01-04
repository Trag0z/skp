#pragma once
#include "Globals.h"

Matrix4 g_finalTransformation = Matrix4();
Matrix4 g_finalRotation = Matrix4();
const SceGxmProgramParameter* g_wvpParam = 0;
const SceGxmProgramParameter* g_rotParam = 0;
const SceGxmProgramParameter* g_localToWorldParam = 0;
const float g_miniCubeHalfSize = 0.5f;
MiniCube* g_miniCubes[3][3][3];

AnimationState g_animationState = ANIMSTATE_NO_ANIM;