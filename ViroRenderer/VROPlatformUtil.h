//
//  VROPlatformUtil.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROPlatformUtil_h
#define VROPlatformUtil_h

#include "VRODefines.h"
#include <string>

std::string VROPlatformGetPathForResource(std::string resource, std::string type);
std::string VROPlatformLoadResourceAsString(std::string resource, std::string type);
std::string VROPlatformLoadFileAsString(std::string path);

#if VRO_PLATFORM_ANDROID
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

void VROPlatformSetAssetManager(JNIEnv *env, jobject assetManager);
#endif

#endif /* VROPlatformUtil_h */
