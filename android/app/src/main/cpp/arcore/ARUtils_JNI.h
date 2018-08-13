//
//  ARUtils_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_ARUTILS_JNI_H
#define ANDROID_ARUTILS_JNI_H

#include <VROARPlaneAnchor.h>
#include "VRODefines.h"
#include VRO_C_INCLUDE

class VROARHitTestResultARCore;

// Helper functions to create Java AR objects from native their representations
VRO_OBJECT ARUtilsCreateJavaARAnchorFromAnchor(std::shared_ptr<VROARAnchor> anchor);
VRO_STRING ARUtilsCreateStringFromAlignment(VROARPlaneAlignment alignment);
VRO_OBJECT ARUtilsCreateARHitTestResult(std::shared_ptr<VROARHitTestResult> result);
VRO_OBJECT ARUtilsCreateARPointCloud(std::shared_ptr<VROARPointCloud> pointCloud);

#endif //ANDROID_ARUTILS_JNI_H
