//
//  VROJenkinsHash.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/21/18.
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

// Modified from JenkensHash.cpp Android Miniken project

/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Implementation of Jenkins one-at-a-time hash function. These choices are
 * optimized for code size and portability, rather than raw speed. But speed
 * should still be quite good.
 **/

#include <stdlib.h>
#include "VROJenkinsHash.h"

namespace android {

#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
uint32_t VROJenkinsHashWhiten(uint32_t hash) {
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

uint32_t VROJenkinsHashMixBytes(uint32_t hash, const uint8_t* bytes, size_t size) {
    if (size > UINT32_MAX) {
        abort();
    }
    hash = VROJenkinsHashMix(hash, (uint32_t)size);
    size_t i;
    for (i = 0; i < (size & -4); i += 4) {
        uint32_t data = bytes[i] | (bytes[i+1] << 8) | (bytes[i+2] << 16) | (bytes[i+3] << 24);
        hash = VROJenkinsHashMix(hash, data);
    }
    if (size & 3) {
        uint32_t data = bytes[i];
        data |= ((size & 3) > 1) ? (bytes[i+1] << 8) : 0;
        data |= ((size & 3) > 2) ? (bytes[i+2] << 16) : 0;
        hash = VROJenkinsHashMix(hash, data);
    }
    return hash;
}

uint32_t VROJenkinsHashMixShorts(uint32_t hash, const uint16_t* shorts, size_t size) {
    if (size > UINT32_MAX) {
        abort();
    }
    hash = VROJenkinsHashMix(hash, (uint32_t)size);
    size_t i;
    for (i = 0; i < (size & -2); i += 2) {
        uint32_t data = shorts[i] | (shorts[i+1] << 16);
        hash = VROJenkinsHashMix(hash, data);
    }
    if (size & 1) {
        uint32_t data = shorts[i];
        hash = VROJenkinsHashMix(hash, data);
    }
    return hash;
}

}

