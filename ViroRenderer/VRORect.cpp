//
//  VRORect.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRORect.h"

VRORect VRORectMake(float x, float y, float width, float height) {
    return { {x, y}, {width, height} };
}