#pragma once
#include <gxm.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;

// TODO: rename to g_cameraTransformation
extern Matrix4 g_finalTransformation;
extern Matrix4 g_finalRotation;
extern const SceGxmProgramParameter *g_wvpParam;
extern const SceGxmProgramParameter *g_rotParam;
extern const SceGxmProgramParameter *g_localToWorldParam;
extern const float g_miniCubeHalfSize;

enum AnimationState { ANIMSTATE_NO_ANIM, ANIMSTATE_ANIMATING, ANIMSTATE_TOUCH };

extern AnimationState g_animationState;