//
//  Polyline_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPolyline.h>
#include <jni.h>
#include <PersistentRef.h>
#include <VROPlatformUtil.h>
#include "Node_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Polyline_##method_name

namespace Polyline {
    inline jlong jptr(std::shared_ptr<VROPolyline> shared_node) {
        PersistentRef<VROPolyline> *native_line = new PersistentRef<VROPolyline>(shared_node);
        return reinterpret_cast<intptr_t>(native_line);
    }

    inline std::shared_ptr<VROPolyline> native(jlong ptr) {
        PersistentRef<VROPolyline> *persistentLine = reinterpret_cast<PersistentRef<VROPolyline> *>(ptr);
        return persistentLine->get();
    }

    VROVector3f convertPoint(JNIEnv *env, jfloatArray point_j) {
        int numCoordinates = env->GetArrayLength(point_j);
        jfloat *point_c = env->GetFloatArrayElements(point_j, 0);

        VROVector3f point;
        if (numCoordinates == 2) {
            point = { point_c[0], point_c[1] };
        }
        else if (numCoordinates >= 3) {
            point = { point_c[0], point_c[1], point_c[2] };
        }
        env->ReleaseFloatArrayElements(point_j, point_c, JNI_ABORT);
        return point;
    }

    std::vector<VROVector3f> convertPoints(JNIEnv *env, jobjectArray points_j) {
        std::vector<VROVector3f> points;
        int numPoints = env->GetArrayLength(points_j);
        for (int i = 0; i < numPoints; i++) {
            jfloatArray point_j = (jfloatArray)env->GetObjectArrayElement(points_j, i);
            points.push_back(convertPoint(env, point_j));
        }
        return points;
    }
}

extern "C" {

JNI_METHOD(jlong, nativeCreatePolylineEmpty)(JNIEnv *env,
                                            jclass clazz,
                                            jfloat width) {

    std::shared_ptr<VROPolyline> polyline = std::make_shared<VROPolyline>();
    polyline->setThickness(width);
    return Polyline::jptr(polyline);
}

JNI_METHOD(jlong, nativeCreatePolyline)(JNIEnv *env,
                                        jclass clazz,
                                        jobjectArray points_j,
                                        jfloat width) {
    std::vector<VROVector3f> points = Polyline::convertPoints(env, points_j);
    std::shared_ptr<VROPolyline> polyline = VROPolyline::createPolyline(points, width);
    return Polyline::jptr(polyline);
}

JNI_METHOD(void, nativeDestroyPolyline)(JNIEnv *env,
                                        jclass clazz,
                                        jlong nativePolylineRef) {
    delete reinterpret_cast<PersistentRef<VROPolyline> *>(nativePolylineRef);
}

JNI_METHOD(void, nativeAppendPoint)(JNIEnv *env,
                                       jclass clazz,
                                       jlong polyline_j,
                                       jfloatArray point_j) {
    std::weak_ptr<VROPolyline> polyline_w = Polyline::native(polyline_j);

    VROVector3f point = Polyline::convertPoint(env, point_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, point] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            polyline->appendPoint(point);
        }
    });
}

JNI_METHOD(void, nativeSetPoints)(JNIEnv *env,
                                  jclass clazz,
                                  jlong polyline_j,
                                  jobjectArray points_j) {
    std::vector<VROVector3f> points = Polyline::convertPoints(env, points_j);

    std::weak_ptr<VROPolyline> polyline_w = Polyline::native(polyline_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, points] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            std::vector<std::vector<VROVector3f>> paths = { points };
            polyline->setPaths(paths);
        }
    });
}

JNI_METHOD(void, nativeSetThickness)(JNIEnv *env,
                                 jclass clazz,
                                 jlong polyline_j,
                                 jfloat thickness) {
    std::weak_ptr<VROPolyline> polyline_w = Polyline::native(polyline_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, thickness] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            polyline->setThickness(thickness);
        }
    });
}

}  // extern "C"
