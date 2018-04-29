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

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Polyline_##method_name
#endif

namespace Polyline {
    inline VRO_REF jptr(std::shared_ptr<VROPolyline> shared_node) {
        PersistentRef<VROPolyline> *native_line = new PersistentRef<VROPolyline>(shared_node);
        return reinterpret_cast<intptr_t>(native_line);
    }

    inline std::shared_ptr<VROPolyline> native(VRO_REF ptr) {
        PersistentRef<VROPolyline> *persistentLine = reinterpret_cast<PersistentRef<VROPolyline> *>(ptr);
        return persistentLine->get();
    }

    VROVector3f convertPoint(VRO_ENV env, VRO_FLOAT_ARRAY point_j) {
        int numCoordinates = VRO_ARRAY_LENGTH(point_j);
        VRO_FLOAT *point_c = VRO_FLOAT_ARRAY_GET_ELEMENTS(point_j);

        VROVector3f point;
        if (numCoordinates == 2) {
            point = { point_c[0], point_c[1] };
        }
        else if (numCoordinates >= 3) {
            point = { point_c[0], point_c[1], point_c[2] };
        }
        VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(point_j, point_c);
        return point;
    }

    std::vector<VROVector3f> convertPoints(VRO_ENV env, VRO_ARRAY points_j) {
        std::vector<VROVector3f> points;
        int numPoints = VRO_ARRAY_LENGTH(points_j);
        for (int i = 0; i < numPoints; i++) {
            VRO_FLOAT_ARRAY point_j = (VRO_FLOAT_ARRAY) VRO_ARRAY_GET(points_j, i);
            points.push_back(convertPoint(env, point_j));
        }
        return points;
    }
}

extern "C" {

VRO_METHOD(VRO_REF, nativeCreatePolylineEmpty)(VRO_ARGS
                                               VRO_FLOAT width) {

    std::shared_ptr<VROPolyline> polyline = std::make_shared<VROPolyline>();
    polyline->setThickness(width);
    return Polyline::jptr(polyline);
}

VRO_METHOD(VRO_REF, nativeCreatePolyline)(VRO_ARGS
                                          VRO_ARRAY points_j,
                                          VRO_FLOAT width) {
    std::vector<VROVector3f> points = Polyline::convertPoints(env, points_j);
    std::shared_ptr<VROPolyline> polyline = VROPolyline::createPolyline(points, width);
    return Polyline::jptr(polyline);
}

VRO_METHOD(void, nativeDestroyPolyline)(VRO_ARGS
                                        VRO_REF nativePolylineRef) {
    delete reinterpret_cast<PersistentRef<VROPolyline> *>(nativePolylineRef);
}

VRO_METHOD(void, nativeAppendPoint)(VRO_ARGS
                                    VRO_REF polyline_j,
                                    VRO_FLOAT_ARRAY point_j) {
    std::weak_ptr<VROPolyline> polyline_w = Polyline::native(polyline_j);

    VROVector3f point = Polyline::convertPoint(env, point_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, point] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            polyline->appendPoint(point);
        }
    });
}

VRO_METHOD(void, nativeSetPoints)(VRO_ARGS
                                  VRO_REF polyline_j,
                                  VRO_ARRAY points_j) {
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

VRO_METHOD(void, nativeSetThickness)(VRO_ARGS
                                     VRO_REF polyline_j,
                                     VRO_FLOAT thickness) {
    std::weak_ptr<VROPolyline> polyline_w = Polyline::native(polyline_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, thickness] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            polyline->setThickness(thickness);
        }
    });
}

}  // extern "C"
