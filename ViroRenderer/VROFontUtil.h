//
//  VROFontUtil.h
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

// Modified from FontUtils.h in Android Miniken

/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef VROFontUtil_h
#define VROFontUtil_h

#include <unordered_set>

constexpr uint32_t MAX_UNICODE_CODE_POINT = 0x10FFFF;
constexpr uint32_t VS1 = 0xFE00;
constexpr uint32_t VS16 = 0xFE0F;
constexpr uint32_t VS17 = 0xE0100;
constexpr uint32_t VS256 = 0xE01EF;

// Returns variation selector index. This is one unit less than the variation selector number. For
// example, VARIATION SELECTOR-25 maps to 24.
// [0x00-0x0F] for U+FE00..U+FE0F
// [0x10-0xFF] for U+E0100..U+E01EF
// kInvalidVSIndex for other input.
constexpr uint16_t kInvalidVSIndex = 0xFFFF;

class VROFontUtil {
public:
    
    static uint16_t getVsIndex(uint32_t codePoint);
    static bool isVariationSelector(uint32_t codePoint);

    // Characters where we want to continue using existing font run for (or stick to the next run if
    // they start a string), even if the font does not support them explicitly. These are handled
    // properly by Minikin or HarfBuzz even if the font does not explicitly support them and it's
    // usually meaningless to switch to a different font to display them.
    static bool charDoesNotNeedFontSupport(uint32_t codePoint);

    // Characters where we want to continue using existing font run instead of
    // recomputing the best match in the fallback list.
    static bool charIsStickyWhitelisted(uint32_t codePoint);
    
    bool analyzeStyle(const uint8_t* os2_data, size_t os2_size, int* weight, bool* italic);
    void analyzeAxes(const uint8_t* fvar_data, size_t fvar_size, std::unordered_set<uint32_t>* axes);
    
};

#endif  // VROFontUtil_h
