//
//  VROPlatformUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROPlatformUtil.h"

#if VRO_PLATFORM_IOS

#import <Foundation/Foundation.h>

std::string VROPlatformGetPathForResource(std::string resource, std::string type) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:[NSString stringWithUTF8String:resource.c_str()]
                                      ofType:[NSString stringWithUTF8String:type.c_str()]];
    
    return std::string([path UTF8String]);
}

std::string VROPlatformLoadFileAsString(std::string path) {
    return std::string([[NSString stringWithContentsOfFile:[NSString stringWithUTF8String:path.c_str()]
                                                  encoding:NSUTF8StringEncoding
                                                     error:nil] UTF8String]);
}

#elif VRO_PLATFORM_ANDROID

std::string VROPlatformGetPathForResource(std::string resource, std::string type) {
    // TODO Android
    return "";
}

std::string VROPlatformLoadFileAsString(std::string path) {
    // TODO Android
    return "";
}

#endif
