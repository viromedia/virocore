//
//  Created by Raj Advani on 9/10/18.
//  Copyright © 2018 Viro Media. All rights reserved.
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

#include "Submesh_JNI.h"
#include "VROPlatformUtil.h"
#include "VROGeometryUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Submesh_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Geometry_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROGeometryElement), nativeCreateGeometryElement)(VRO_NO_ARGS) {
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>();
    return VRO_REF_NEW(VROGeometryElement, element);
}

VRO_METHOD(void, nativeSetTriangleIndices)(VRO_ARGS
                                           VRO_REF(VROGeometryElement) nativeRef,
                                           VRO_INT_ARRAY indices_j) {
    int *indices_c = VRO_INT_ARRAY_GET_ELEMENTS(indices_j);
    int numIndices = VRO_ARRAY_LENGTH(indices_j);

    int indexSize = sizeof(int);
    std::shared_ptr<VROData> indicesData = std::make_shared<VROData>((void *) indices_c, numIndices * indexSize);

    std::weak_ptr<VROGeometryElement> element_w = VRO_REF_GET(VROGeometryElement, nativeRef);
    VROPlatformDispatchAsyncRenderer([element_w, indicesData, numIndices, indexSize] {
        std::shared_ptr<VROGeometryElement> element = element_w.lock();
        if (element) {
            element->setPrimitiveType(VROGeometryPrimitiveType::Triangle);
            element->setPrimitiveCount(VROGeometryUtilGetPrimitiveCount(numIndices, VROGeometryPrimitiveType::Triangle));
            element->setBytesPerIndex(indexSize);
            element->setData(indicesData);
        }
    });
    VRO_INT_ARRAY_RELEASE_ELEMENTS(indices_j, indices_c);
}

}