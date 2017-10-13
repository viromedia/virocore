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
}

extern "C" {

JNI_METHOD(jlong, nativeCreatePolyline)(JNIEnv *env,
                                        jclass clazz,
                                        jobjectArray points,
                                        jfloat width) {
    std::vector<VROVector3f> nativePoints;
    int numPoints = env->GetArrayLength(points);
    for (int i = 0; i < numPoints; i++) {
        jfloatArray point = (jfloatArray)env->GetObjectArrayElement(points, i);
        jfloat *pointArray = env->GetFloatArrayElements(point, 0);
        VROVector3f nativePoint = VROVector3f(pointArray[0], pointArray[1]);
        nativePoints.push_back(nativePoint);
    }

    std::shared_ptr<VROPolyline> polyline = VROPolyline::createPolyline(nativePoints, width);
    return Polyline::jptr(polyline);
}

JNI_METHOD(void, nativeDestroyPolyline)(JNIEnv *env,
                                        jclass clazz,
                                        jlong nativePolylineRef) {
    delete reinterpret_cast<PersistentRef<VROPolyline> *>(nativePolylineRef);
}

JNI_METHOD(void, nativeSetThickness)(JNIEnv *env,
                                 jclass clazz,
                                 jlong nativePolylineRef,
                                 jfloat thickness) {
    std::weak_ptr<VROPolyline> polyline_w = Polyline::native(nativePolylineRef);
    VROPlatformDispatchAsyncRenderer([polyline_w, thickness] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            polyline->setThickness(thickness);
        }
    });
}

}  // extern "C"
