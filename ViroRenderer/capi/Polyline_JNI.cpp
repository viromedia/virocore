//
//  Polyline_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROPolyline.h"
#include "VROPlatformUtil.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Polyline_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Polyline_##method_name
#endif

namespace Polyline {

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

    std::vector<VROVector3f> convertPoints(VRO_ENV env, VRO_ARRAY(VRO_FLOAT_ARRAY) points_j) {
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

VRO_METHOD(VRO_REF(VROPolyline), nativeCreatePolylineEmpty)(VRO_ARGS
                                                            VRO_FLOAT width) {
    std::shared_ptr<VROPolyline> polyline = std::make_shared<VROPolyline>();
    polyline->setThickness(width);
    return VRO_REF_NEW(VROPolyline, polyline);
}

VRO_METHOD(VRO_REF(VROPolyline), nativeCreatePolyline)(VRO_ARGS
                                                       VRO_ARRAY(VRO_FLOAT_ARRAY) points_j,
                                                       VRO_FLOAT width) {
    VRO_METHOD_PREAMBLE;

    std::vector<VROVector3f> points = Polyline::convertPoints(env, points_j);
    std::shared_ptr<VROPolyline> polyline = VROPolyline::createPolyline(points, width);
    return VRO_REF_NEW(VROPolyline, polyline);
}

VRO_METHOD(void, nativeAppendPoint)(VRO_ARGS
                                    VRO_REF(VROPolyline) polyline_j,
                                    VRO_FLOAT_ARRAY point_j) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<VROPolyline> polyline_w = VRO_REF_GET(VROPolyline, polyline_j);

    VROVector3f point = Polyline::convertPoint(env, point_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, point] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            polyline->appendPoint(point);
        }
    });
}

VRO_METHOD(void, nativeSetPoints)(VRO_ARGS
                                  VRO_REF(VROPolyline) polyline_j,
                                  VRO_ARRAY(VRO_FLOAT_ARRAY) points_j) {
    VRO_METHOD_PREAMBLE;
    std::vector<VROVector3f> points = Polyline::convertPoints(env, points_j);

    std::weak_ptr<VROPolyline> polyline_w = VRO_REF_GET(VROPolyline, polyline_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, points] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            std::vector<std::vector<VROVector3f>> paths = { points };
            polyline->setPaths(paths);
        }
    });
}

VRO_METHOD(void, nativeSetThickness)(VRO_ARGS
                                     VRO_REF(VROPolyline) polyline_j,
                                     VRO_FLOAT thickness) {
    std::weak_ptr<VROPolyline> polyline_w = VRO_REF_GET(VROPolyline, polyline_j);
    VROPlatformDispatchAsyncRenderer([polyline_w, thickness] {
        std::shared_ptr<VROPolyline> polyline = polyline_w.lock();
        if (polyline) {
            polyline->setThickness(thickness);
        }
    });
}

}  // extern "C"
