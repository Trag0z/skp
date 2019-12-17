#pragma once
#include "Globals.h"
#include "MiniCube.h"
#include <cstdio>
#include <libdbg.h>
#include <sce_geometry.h>
#include <touch.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;
using namespace sce::Geometry::Aos;

extern AnimationState g_animationState;
extern Matrix4 g_finalTransformation;
extern const float g_miniCubeHalfSize;

struct TouchData {
    float x;
    float y;
    int id;
};

enum TouchDirection { DIR_NO, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

static SceTouchPanelInfo s_touchInfo;
static TouchData s_lastBackTouch;
static TouchData s_lastFrontTouch;
static int s_touchedSide;
static TouchDirection s_touchMotionDirection = DIR_NO;

static Vector2 s_startPosOnCube = Vector2(0.0f, 0.0f);
static float s_accumulatedTurningAngle[2] = {0.0f, 0.0f};

static const float c_cubeHalfSize = g_miniCubeHalfSize * 3.0f;
static const float c_touchThreshold = 5.0f;
static const float c_touchSensitivity = 0.5f;

static const Plane s_planes[] = {
    Plane(Point3(0.0f, 0.0f, -c_cubeHalfSize), Vector3(0.0f, 0.0f, -1.0f)),
    Plane(Point3(0.0f, 0.0f, c_cubeHalfSize), Vector3(0.0f, 0.0f, 1.0f)),
    Plane(Point3(-c_cubeHalfSize, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f)),
    Plane(Point3(c_cubeHalfSize, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f)),
    Plane(Point3(0.0f, -c_cubeHalfSize, 0.0f), Vector3(0.0f, -1.0f, 1.0f)),
    Plane(Point3(0.0f, c_cubeHalfSize, 0.0f), Vector3(0.0f, 1.0f, 0.0f))};

inline void initTouch() {
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK,
                             SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchGetPanelInfo(SCE_TOUCH_PORT_BACK, &s_touchInfo);

    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,
                             SCE_TOUCH_SAMPLING_STATE_START);
}

inline const Vector3 processBackTouch() {
    SceTouchData tdb;
    // NOTE: maybe the last argument should be 1 because we don't deal with
    // multitouch?
    int returnCode = sceTouchRead(SCE_TOUCH_PORT_BACK, &tdb, 1);
    SCE_DBG_ALWAYS_ASSERT(returnCode <= 1);

    if (tdb.reportNum > 0) {
        SceTouchReport& report = tdb.report[0];
        if (report.id == s_lastBackTouch.id) {
            s_accumulatedTurningAngle[0] +=
                (report.x - s_lastBackTouch.x) * 0.01f;
            s_accumulatedTurningAngle[1] +=
                (report.y - s_lastBackTouch.y) * 0.01f;
        }
        s_lastBackTouch.id = report.id;
        s_lastBackTouch.x = report.x;
        s_lastBackTouch.y = report.y;
    }

    return Vector3(s_accumulatedTurningAngle[1], -s_accumulatedTurningAngle[0],
                   0.0f);
}

inline void processFrontTouch(MiniCube* miniCubes) {
    SceTouchData tdf;
    int returnCode = sceTouchRead(SCE_TOUCH_PORT_FRONT, &tdf, 1);
    SCE_DBG_ALWAYS_ASSERT(returnCode <= 1);

    // Cast ray
    switch (g_animationState) {
    case ANIMSTATE_NO_ANIM:
        // Set up animation start
        if (tdf.reportNum == 0) {
            break;
        }
        s_lastFrontTouch.id = tdf.report[0].id;
        s_lastFrontTouch.x = 2.0f * tdf.report[0].x / 1919.0f - 1.0f;
        // (2.0f * tdf.report[0].x / s_touchInfo.maxAaX - 1.0f) * 4.5f;
        s_lastFrontTouch.y = -2.0f * tdf.report[0].y / 1087.0f + 1.0f;
        // (2.0f * tdf.report[0].y / s_touchInfo.maxAaY - 1.0f) * 2.5f;

        Matrix4 inverseFinalTransform = inverse(g_finalTransformation);
        Vector4 p1 =
            inverseFinalTransform *
            Vector4(s_lastFrontTouch.x, s_lastFrontTouch.y, 0.1f, 1.0f);
        Vector4 p2 =
            inverseFinalTransform *
            Vector4(s_lastFrontTouch.x, s_lastFrontTouch.y, 0.9f, 1.0f);

        std::printf(
            "Before:\np1: %.2f %.2f %.2f %.2f\np2: %.2f %.2f %.2f %.2f\n",
            p1.getX().getAsFloat(), p1.getY().getAsFloat(),
            p1.getZ().getAsFloat(), p1.getW().getAsFloat(),
            p2.getX().getAsFloat(), p2.getY().getAsFloat(),
            p2.getZ().getAsFloat(), p2.getW().getAsFloat());

        p1 /= p1.getW();
        p2 /= p2.getW();

        std::printf(
            "After:\np1: %.2f %.2f %.2f %.2f\np2: %.2f %.2f %.2f %.2f\n",
            p1.getX().getAsFloat(), p1.getY().getAsFloat(),
            p1.getZ().getAsFloat(), p1.getW().getAsFloat(),
            p2.getX().getAsFloat(), p2.getY().getAsFloat(),
            p2.getZ().getAsFloat(), p2.getW().getAsFloat());

        Ray ray = Ray(Point3(p1.getXYZ()), Point3(p2.getXYZ()));

        Point3 intersection;
        for (int i = 0; i < 6; ++i) {
            if (!intersectionPoint(ray, s_planes[i], &intersection)) {
                continue;
            }

            const float x = intersection.getX();
            const float y = intersection.getY();
            const float z = intersection.getZ();

            if (i == 0) {
                std::printf("Hit %d:\n%.2f %.2f %.2f\n", i, x, y, z);
            }
            if (x < -c_cubeHalfSize || x > c_cubeHalfSize ||
                y < -c_cubeHalfSize || y > c_cubeHalfSize ||
                z < -c_cubeHalfSize || z > c_cubeHalfSize ||
                dot(ray.getDirection(), s_planes[i].getNormal()).getAsFloat() >
                    0.0f) {
                // We didn't hit the cube, or we hit its back side
                continue;
            }

            s_touchedSide = i;
            // startPosOnCube always from top right of first mentioned side
            if (s_touchedSide == 0 || s_touchedSide == 1)
                s_startPosOnCube = Vector2(x, y);
            else if (s_touchedSide == 2 || s_touchedSide == 3)
                s_startPosOnCube = Vector2(-z, y);
            else
                s_startPosOnCube = Vector2(x, -z);
            break;
        }
        break;
    case ANIMSTATE_TOUCH:
        // Continue animation
        bool touchReleased = true;
        for (int i = 0; i < tdf.reportNum; ++i) {
            if (tdf.report[i].id != s_lastFrontTouch.id) {
                continue;
            }

            Matrix4 inverseFinalTransform = inverse(g_finalTransformation);
            Vector4 p1 =
                inverseFinalTransform *
                Vector4(s_lastFrontTouch.x, s_lastFrontTouch.y, 0.1f, 1.0f);
            Vector4 p2 =
                inverseFinalTransform *
                Vector4(s_lastFrontTouch.x, s_lastFrontTouch.y, 1.0f, 1.0f);

            Ray ray = Ray(Point3(p1.getXYZ()), Point3(p2.getXYZ()));

            Point3 intersection;
            if (!intersectionPoint(ray, s_planes[s_touchedSide],
                                   &intersection)) {
                // Touch is not on the same side of the cube anymore
                continue;
            }
            touchReleased = false;

            Vector2 newTouchPos;

            if (s_touchedSide == 0 || s_touchedSide == 1)
                newTouchPos = Vector2(intersection.getX(), intersection.getY());
            else if (s_touchedSide == 2 || s_touchedSide == 3)
                newTouchPos =
                    Vector2(-intersection.getZ(), intersection.getY());
            else
                newTouchPos =
                    Vector2(intersection.getX(), -intersection.getZ());

            Vector2 touchMotion = newTouchPos - s_startPosOnCube;
            float touchMotionLength = length(touchMotion).getAsFloat();

            if (touchMotionLength < c_touchThreshold) {
                continue;
            }

            TouchDirection newTouchMotionDirection;
            if (fabsf(touchMotion.getX()) > fabsf(touchMotion.getY())) {
                if (touchMotion.getX() < 0) {
                    newTouchMotionDirection = DIR_LEFT;
                } else {
                    newTouchMotionDirection = DIR_RIGHT;
                }
            } else {
                if (touchMotion.getY() < 0) {
                    newTouchMotionDirection = DIR_UP;
                } else {
                    newTouchMotionDirection = DIR_DOWN;
                }
            }

            // If the direction is already set and we didn't change to the
            // opposite one, set interpolation value
            if (s_touchMotionDirection != DIR_NO &&
                (!(s_touchMotionDirection == DIR_LEFT &&
                   newTouchMotionDirection == DIR_RIGHT) ||
                 !(s_touchMotionDirection == DIR_RIGHT &&
                   newTouchMotionDirection == DIR_LEFT) ||
                 !(s_touchMotionDirection == DIR_UP &&
                   newTouchMotionDirection == DIR_DOWN) ||
                 !(s_touchMotionDirection == DIR_DOWN &&
                   newTouchMotionDirection == DIR_UP))) {
                float interpolationValue =
                    touchMotionLength * c_touchSensitivity;
                setAnimationInterpolationValue(interpolationValue);
                return;
            }

            // Determine layer and rotation direction
            if (newTouchMotionDirection == DIR_LEFT ||
                newTouchMotionDirection == DIR_RIGHT) {
                // Vertical motion
                if (s_startPosOnCube.getX() < -g_miniCubeHalfSize) {
                    // Left column
                    if (newTouchMotionDirection == 1) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 0, DIM_X, false);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 0, DIM_X, true);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 2, DIM_Z, true);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 2, DIM_Z, false);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 0, DIM_X, false);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 0, DIM_X, true);
                    } else if (newTouchMotionDirection == 2) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 0, DIM_X, true);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 0, DIM_X, false);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 2, DIM_Z, false);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 2, DIM_Z, true);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 0, DIM_X, true);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 0, DIM_X, false);
                    }
                } else if (s_startPosOnCube.getX() > g_miniCubeHalfSize) {
                    // Right column
                    if (newTouchMotionDirection == 1) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 2, DIM_X, false);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 2, DIM_X, true);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 0, DIM_Z, true);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 0, DIM_Z, false);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 2, DIM_X, false);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 2, DIM_X, true);
                    } else if (newTouchMotionDirection == 2) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 2, DIM_X, true);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 2, DIM_X, false);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 0, DIM_Z, false);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 0, DIM_Z, true);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 2, DIM_X, true);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 2, DIM_X, false);
                    }
                } else {
                    // Middle column
                    if (newTouchMotionDirection == 1) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 1, DIM_X, false);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 1, DIM_X, true);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 1, DIM_Z, true);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 1, DIM_Z, false);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 1, DIM_X, false);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 1, DIM_X, true);
                    } else if (newTouchMotionDirection == 2) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 1, DIM_X, true);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 1, DIM_X, false);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 1, DIM_Z, false);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 1, DIM_Z, true);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 1, DIM_X, true);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 1, DIM_X, false);
                    }
                }
            } else {
                // Horizontal motion
                if (s_startPosOnCube.getY() < -g_miniCubeHalfSize) {
                    // Top row
                    if (newTouchMotionDirection == 3) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 0, DIM_Y, true);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 0, DIM_Y, false);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 0, DIM_Y, true);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 0, DIM_Y, false);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 2, DIM_Z, false);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 2, DIM_Z, true);
                    } else if (newTouchMotionDirection == 4) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 0, DIM_Y, false);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 0, DIM_Y, true);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 0, DIM_Y, false);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 0, DIM_Y, true);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 2, DIM_Z, true);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 2, DIM_Z, false);
                    }
                } else if (s_startPosOnCube.getY() > g_miniCubeHalfSize) {
                    // Bottom row
                    if (newTouchMotionDirection == 3) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 2, DIM_Y, true);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 2, DIM_Y, false);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 2, DIM_Y, true);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 2, DIM_Y, false);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 0, DIM_Z, false);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 0, DIM_Z, true);
                    } else if (newTouchMotionDirection == 4) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 2, DIM_Y, false);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 2, DIM_Y, true);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 2, DIM_Y, false);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 2, DIM_Y, true);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 0, DIM_Z, true);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 0, DIM_Z, false);
                    }
                } else {
                    // Middle row
                    if (newTouchMotionDirection == 3) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 1, DIM_Y, true);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 1, DIM_Y, false);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 1, DIM_Y, true);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 1, DIM_Y, false);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 1, DIM_Z, false);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 1, DIM_Z, true);
                    } else if (newTouchMotionDirection == 4) {
                        if (s_touchedSide == 0)
                            setAnimation(miniCubes, 1, DIM_Y, false);
                        else if (s_touchedSide == 1)
                            setAnimation(miniCubes, 1, DIM_Y, true);
                        else if (s_touchedSide == 2)
                            setAnimation(miniCubes, 1, DIM_Y, false);
                        else if (s_touchedSide == 3)
                            setAnimation(miniCubes, 1, DIM_Y, true);
                        else if (s_touchedSide == 4)
                            setAnimation(miniCubes, 1, DIM_Z, true);
                        else if (s_touchedSide == 5)
                            setAnimation(miniCubes, 1, DIM_Z, false);
                    }
                }
            }
        }

        if (touchReleased) {
            g_animationState = ANIMSTATE_ANIMATING;
        }
        break;
    case ANIMSTATE_ANIMATING:
        break;
    default:
        SCE_DBG_ALWAYS_ASSERT(false);
        break;
    }
}