//
//  VROStringUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROStringUtil.h"
#include <sstream>

std::string VROStringUtil::toString(int i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}
