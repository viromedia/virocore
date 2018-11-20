//
//  VRODriver.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
#include "VRODriver.h"

// Note this class has limited functionality (most is in the header) but we still require a
// cpp file in order to have a 'key function' which guarantees we get a strong global symbol
// for this class in the typeinfo of the library, so that dynamic_cast works across dlopen
// boundaries.
//
// See here: https://github.com/android-ndk/ndk/issues/533#issuecomment-335977747
VRODriver::VRODriver() {

}

VRODriver::~VRODriver() {

}