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
std::string VROPlatformLoadFileAsString(std::string path);

#endif /* VROPlatformUtil_h */
