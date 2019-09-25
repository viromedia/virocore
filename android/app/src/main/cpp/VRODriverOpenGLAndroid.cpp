//
//  VRODriverOpenGLAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/19/18.
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

#include "VRODriverOpenGLAndroid.h"

// Note this class has limited functionality (most is in the header) but we still require a
// cpp file in order to have a 'key function' which guarantees we get a strong global symbol
// for this class in the typeinfo of the library, so that dynamic_cast works across dlopen
// boundaries.
//
// See here: https://github.com/android-ndk/ndk/issues/533#issuecomment-335977747
VRODriverOpenGLAndroid::VRODriverOpenGLAndroid(std::shared_ptr<gvr::AudioApi> gvrAudio) :
        _gvrAudio(gvrAudio),
        _ft(nullptr) {
}

VRODriverOpenGLAndroid::~VRODriverOpenGLAndroid() {
    if (_ft != nullptr) {
        FT_Done_FreeType(_ft);
        _ft = nullptr;
    }
}