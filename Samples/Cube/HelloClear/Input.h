#pragma once
#include "Globals.h"
#include "MiniCube.h"
#include <libdbg.h>
#include <sce_geometry.h>
#include <touch.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;
using namespace sce::Geometry::Aos;

extern AnimationState g_animationState;
extern Matrix4 g_cameraTransformation;
extern const float g_miniCubeHalfSize;
extern bool g_animationStarted;

struct TouchData {
    float x;
    float y;
    int id;
};

enum TouchDirection { DIR_NO, DIR_HORIZONTAL, DIR_VERTICAL };

static SceTouchPanelInfo s_touchInfo;
static TouchData s_lastBackTouch;
static TouchData s_lastFrontTouch;
static int s_touchedSide;
static TouchDirection s_touchMotionDirection = DIR_NO;

static Vector2 s_startPosOnCube = Vector2(0.0f, 0.0f);

static const float c_cubeHalfSize = g_miniCubeHalfSize * 3.0f;
static const float c_touchThreshold = 0.15f;
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

inline const Vector2 processBackTouch() {
    SceTouchData tdb;
    int returnCode = sceTouchRead(SCE_TOUCH_PORT_BACK, &tdb, 1);
    SCE_DBG_ALWAYS_ASSERT(returnCode <= 1);

    Vector2 move = Vector2(0.0f, 0.0f);
    if (tdb.reportNum > 0) {
        SceTouchReport& report = tdb.report[0];
        if (report.id == s_lastBackTouch.id) {
            move = Vector2(report.x - s_lastBackTouch.x,
                           report.y - s_lastBackTouch.y);
        }
        s_lastBackTouch.id = report.id;
        s_lastBackTouch.x = report.x;
        s_lastBackTouch.y = report.y;
    }

    return move;
}

void determineNewAnimation() {
    // Determine layer and rotation direction
    if (s_touchMotionDirection == DIR_VERTICAL) {
        // Vertical motion
        if (s_startPosOnCube.getX() < -g_miniCubeHalfSize) {
            // Left column
            if (s_touchedSide == 0 || s_touchedSide == 1)
                setAnimation(0, DIM_X);
            else if (s_touchedSide == 2 || s_touchedSide == 3)
                setAnimation(2, DIM_Z);
            else if (s_touchedSide == 4 || s_touchedSide == 5)
                setAnimation(0, DIM_X);
        } else if (s_startPosOnCube.getX() > g_miniCubeHalfSize) {
            // Right column
            if (s_touchedSide == 0 || s_touchedSide == 1 ||
                s_touchedSide == 4 || s_touchedSide == 5)
                setAnimation(2, DIM_X);
            else if (s_touchedSide == 2 || s_touchedSide == 3)
                setAnimation(0, DIM_Z);
        } else {
            // Middle column
            if (s_touchedSide == 0 || s_touchedSide == 1 ||
                s_touchedSide == 4 || s_touchedSide == 5)
                setAnimation(1, DIM_X);
            else if (s_touchedSide == 2 || s_touchedSide == 3)
                setAnimation(1, DIM_Z);
        }
    } else {
        // Horizontal motion
        if (s_startPosOnCube.getY() < -g_miniCubeHalfSize) {
            // Top row
            if (s_touchedSide == 0 || s_touchedSide == 1 ||
                s_touchedSide == 2 || s_touchedSide == 3)
                setAnimation(0, DIM_Y);
            else if (s_touchedSide == 4 || s_touchedSide == 5)
                setAnimation(2, DIM_Z);
        } else if (s_startPosOnCube.getY() > g_miniCubeHalfSize) {
            // Bottom row
            if (s_touchedSide == 0 || s_touchedSide == 1 ||
                s_touchedSide == 2 || s_touchedSide == 3)
                setAnimation(2, DIM_Y);
            else if (s_touchedSide == 4 || s_touchedSide == 5)
                setAnimation(0, DIM_Z);
        } else {
            // Middle row
            if (s_touchedSide == 0 || s_touchedSide == 1 | s_touchedSide == 2 ||
                s_touchedSide == 3)
                setAnimation(1, DIM_Y);
            else if (s_touchedSide == 4 || s_touchedSide == 5)
                setAnimation(1, DIM_Z);
        }
    }
}

inline void processFrontTouch() {
    SceTouchData tdf;
    int returnCode = sceTouchRead(SCE_TOUCH_PORT_FRONT, &tdf, 6);
    SCE_DBG_ALWAYS_ASSERT(returnCode >= 0);
    bool touchReleased = true;

    switch (g_animationState) {
    case ANIMSTATE_NO_ANIM:
        // Set up animation start
        if (tdf.reportNum == 0) {
            break;
        }
        s_lastFrontTouch.id = tdf.report[0].id;
        s_lastFrontTouch.x = 2.0f * tdf.report[0].x / 1919.0f - 1.0f;
        s_lastFrontTouch.y = -2.0f * tdf.report[0].y / 1087.0f + 1.0f;

        Matrix4 inverseFinalTransform = inverse(g_cameraTransformation);
        Vector4 p1 =
            inverseFinalTransform *
            Vector4(s_lastFrontTouch.x, s_lastFrontTouch.y, 0.1f, 1.0f);
        Vector4 p2 =
            inverseFinalTransform *
            Vector4(s_lastFrontTouch.x, s_lastFrontTouch.y, 1.0f, 1.0f);

        p1 /= p1.getW();
        p2 /= p2.getW();

        Ray ray = Ray(Point3(p1.getXYZ()), Point3(p2.getXYZ()));

        Point3 intersection;
        for (int i = 0; i < 6; ++i) {
            if (dot(ray.getDirection(), s_planes[i].getNormal()).getAsFloat() >
                0.0f) {
                continue;
            }
            if (!intersectionPoint(ray, s_planes[i], &intersection)) {
                continue;
            }

            const float x = intersection.getX();
            const float y = intersection.getY();
            const float z = intersection.getZ();
            // NOTE: Only on cubeSide 4, the intersection point is sometimes
            // outside of the cube dimensions even though the physical touch
            // point is not
            if (x < -c_cubeHalfSize || x > c_cubeHalfSize ||
                y < -c_cubeHalfSize || y > c_cubeHalfSize ||
                z < -c_cubeHalfSize || z > c_cubeHalfSize) {
                // The ray didn't hit the cube
                continue;
            }

            s_touchedSide = i;
            // Origin of s_startPosOnCube in top left corner of the slice,
            // viewed from front/top/left depending on dimension
            if (s_touchedSide == 0 || s_touchedSide == 1) {
                s_startPosOnCube = Vector2(x, y);
            } else if (s_touchedSide == 2 || s_touchedSide == 3) {
                s_startPosOnCube = Vector2(-z, y);
            } else {
                s_startPosOnCube = Vector2(x, -z);
            }
            g_animationState = ANIMSTATE_TOUCHING;
            break;
        }
        break;
    case ANIMSTATE_TOUCHING:
        for (int i = 0; i < tdf.reportNum; ++i) {
            if (tdf.report[i].id != s_lastFrontTouch.id) {
                continue;
            }

            float newTouchPos[2];
            newTouchPos[0] = 2.0f * tdf.report[0].x / 1919.0f - 1.0f;
            newTouchPos[1] = -2.0f * tdf.report[0].y / 1087.0f + 1.0f;

            Matrix4 inverseFinalTransform = inverse(g_cameraTransformation);
            Vector4 p1 = inverseFinalTransform *
                         Vector4(newTouchPos[0], newTouchPos[1], 0.1f, 1.0f);
            Vector4 p2 = inverseFinalTransform *
                         Vector4(newTouchPos[0], newTouchPos[1], 1.0f, 1.0f);

            p1 /= p1.getW();
            p2 /= p2.getW();

            Ray ray = Ray(Point3(p1.getXYZ()), Point3(p2.getXYZ()));

            Point3 intersection;
            if (!intersectionPoint(ray, s_planes[s_touchedSide],
                                   &intersection)) {
                // Touch is not on the same side of the cube anymore
                continue;
            }
            touchReleased = false;

            Vector2 newTouchPosOnCube;

            if (s_touchedSide == 0 || s_touchedSide == 1) {
                newTouchPosOnCube =
                    Vector2(intersection.getX(), intersection.getY());
            } else if (s_touchedSide == 2 || s_touchedSide == 3) {
                newTouchPosOnCube =
                    Vector2(-intersection.getZ(), intersection.getY());
            } else {
                newTouchPosOnCube =
                    Vector2(intersection.getX(), -intersection.getZ());
            }

            Vector2 touchMotion = newTouchPosOnCube - s_startPosOnCube;

            if (!g_animationStarted) {
                if (std::abs(length(touchMotion).getAsFloat()) <
                    c_touchThreshold) {
                    continue;
                }

                // Set s_touchMotionDirection, animate in following
                // frames
                g_animationStarted = true;
                if (fabsf(touchMotion.getX()) > fabsf(touchMotion.getY())) {
                    s_touchMotionDirection = DIR_HORIZONTAL;
                } else {
                    s_touchMotionDirection = DIR_VERTICAL;
                }
                determineNewAnimation();
            }

            float touchMotionLength;
            if (s_touchMotionDirection == DIR_HORIZONTAL) {
                touchMotionLength = touchMotion.getX().getAsFloat();
            }
            if (s_touchMotionDirection == DIR_VERTICAL) {
                touchMotionLength = touchMotion.getY().getAsFloat();
            }

            float interpolation = touchMotionLength * c_touchSensitivity / 4.0f;
            if ((s_touchMotionDirection == DIR_HORIZONTAL &&
                 s_touchedSide == 0) ||
                (s_touchMotionDirection == DIR_VERTICAL &&
                 s_touchedSide == 1) ||
                s_touchedSide == 2 || s_touchedSide == 5) {
                interpolation *= -1.0f;
            }

            setAnimationInterpolationValue(interpolation);
        }

        if (touchReleased) {
            g_animationState = ANIMSTATE_ANIMATING;
            s_touchMotionDirection = DIR_NO;
            g_animationStarted = false;
        }
        break;

    case ANIMSTATE_ANIMATING:
        break;
    default:
        SCE_DBG_ALWAYS_ASSERT(false);
        break;
    }
}