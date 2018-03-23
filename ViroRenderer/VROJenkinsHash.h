//
//  VROJenkinsHash.h
//  ViroKit
//
//  Created by Raj Advani on 3/21/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

// Modified from JenkinsHash.h in Android Miniken project

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

#ifndef VROJenkinsHash_h
#define VROJenkinsHash_h

namespace android {

/* The Jenkins hash of a sequence of 32 bit words A, B, C is:
 * Whiten(Mix(Mix(Mix(0, A), B), C)) */

#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
inline uint32_t VROJenkinsHashMix(uint32_t hash, uint32_t data) {
    hash += data;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}

uint32_t VROJenkinsHashWhiten(uint32_t hash);

/* Helpful utility functions for hashing data in 32 bit chunks */
uint32_t VROJenkinsHashMixBytes(uint32_t hash, const uint8_t* bytes, size_t size);

uint32_t VROJenkinsHashMixShorts(uint32_t hash, const uint16_t* shorts, size_t size);

}

#endif // VROJenkinsHash_h
