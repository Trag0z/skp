#pragma once
#include "Globals.h"

Matrix4 g_finalTransformation = Matrix4();
Matrix4 g_finalRotation = Matrix4();
const SceGxmProgramParameter *g_wvpParam = NULL;
const SceGxmProgramParameter *g_rotParam = NULL;
const SceGxmProgramParameter *g_localToWorldParam = NULL;
const float g_miniCubeHalfSize = 0.5f;

AnimationState g_animationState = ANIMSTATE_NO_ANIM;