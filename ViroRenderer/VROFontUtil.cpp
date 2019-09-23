//
//  VROFontUtil.cpp
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

// Modified from FontUtils.cpp in Android Miniken

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

#include "VROFontUtil.h"
#include <stdlib.h>
#include <stdint.h>

static uint16_t readU16(const uint8_t* data, size_t offset) {
    return data[offset] << 8 | data[offset + 1];
}

static uint32_t readU32(const uint8_t* data, size_t offset) {
    return ((uint32_t)data[offset]) << 24 | ((uint32_t)data[offset + 1]) << 16 |
            ((uint32_t)data[offset + 2]) << 8 | ((uint32_t)data[offset + 3]);
}

bool VROFontUtil::analyzeStyle(const uint8_t* os2_data, size_t os2_size, int* weight, bool* italic) {
    const size_t kUsWeightClassOffset = 4;
    const size_t kFsSelectionOffset = 62;
    const uint16_t kItalicFlag = (1 << 0);
    if (os2_size < kFsSelectionOffset + 2) {
        return false;
    }
    uint16_t weightClass = readU16(os2_data, kUsWeightClassOffset);
    *weight = weightClass / 100;
    uint16_t fsSelection = readU16(os2_data, kFsSelectionOffset);
    *italic = (fsSelection & kItalicFlag) != 0;
    return true;
}

void VROFontUtil::analyzeAxes(const uint8_t* fvar_data, size_t fvar_size, std::unordered_set<uint32_t>* axes) {
    const size_t kMajorVersionOffset = 0;
    const size_t kMinorVersionOffset = 2;
    const size_t kOffsetToAxesArrayOffset = 4;
    const size_t kAxisCountOffset = 8;
    const size_t kAxisSizeOffset = 10;

    axes->clear();

    if (fvar_size < kAxisSizeOffset + 2) {
        return;
    }
    const uint16_t majorVersion = readU16(fvar_data, kMajorVersionOffset);
    const uint16_t minorVersion = readU16(fvar_data, kMinorVersionOffset);
    const uint32_t axisOffset = readU16(fvar_data, kOffsetToAxesArrayOffset);
    const uint32_t axisCount = readU16(fvar_data, kAxisCountOffset);
    const uint32_t axisSize = readU16(fvar_data, kAxisSizeOffset);

    if (majorVersion != 1 || minorVersion != 0 || axisOffset != 0x10 || axisSize != 0x14) {
        return;  // Unsupported version.
    }
    if (fvar_size < axisOffset + axisOffset * axisCount) {
        return;  // Invalid table size.
    }
    for (uint32_t i = 0; i < axisCount; ++i) {
        size_t axisRecordOffset = axisOffset + i * axisSize;
        uint32_t tag = readU32(fvar_data, axisRecordOffset);
        axes->insert(tag);
    }
}
    
bool isBMPVariationSelector(uint32_t codePoint) {
    return VS1 <= codePoint && codePoint <= VS16;
}

inline static bool isVariationSelectorSupplement(uint32_t codePoint) {
    return VS17 <= codePoint && codePoint <= VS256;
}

uint16_t VROFontUtil::getVsIndex(uint32_t codePoint) {
    if (isBMPVariationSelector(codePoint)) {
        return codePoint - VS1;
    } else if (isVariationSelectorSupplement(codePoint)) {
        return codePoint - VS17 + 16;
    } else {
        return kInvalidVSIndex;
    }
}

bool VROFontUtil::isVariationSelector(uint32_t codePoint) {
    return isBMPVariationSelector(codePoint) || isVariationSelectorSupplement(codePoint);
}

bool VROFontUtil::charDoesNotNeedFontSupport(uint32_t c) {
    return c == 0x00AD // SOFT HYPHEN
    || c == 0x034F // COMBINING GRAPHEME JOINER
    || c == 0x061C // ARABIC LETTER MARK
    || (0x200C <= c && c <= 0x200F) // ZERO WIDTH NON-JOINER..RIGHT-TO-LEFT MARK
    || (0x202A <= c && c <= 0x202E) // LEFT-TO-RIGHT EMBEDDING..RIGHT-TO-LEFT OVERRIDE
    || (0x2066 <= c && c <= 0x2069) // LEFT-TO-RIGHT ISOLATE..POP DIRECTIONAL ISOLATE
    || c == 0xFEFF // BYTE ORDER MARK
    || isVariationSelector(c);
}

// Characters where we want to continue using existing font run instead of
// recomputing the best match in the fallback list.
static const uint32_t stickyWhitelist[] = {
    '!',
    ',',
    '-',
    '.',
    ':',
    ';',
    '?',
    0x00A0, // NBSP
    0x2010, // HYPHEN
    0x2011, // NB_HYPHEN
    0x202F, // NNBSP
    0x2640, // FEMALE_SIGN,
    0x2642, // MALE_SIGN,
    0x2695, // STAFF_OF_AESCULAPIUS
};

bool VROFontUtil::charIsStickyWhitelisted(uint32_t c) {
    for (size_t i = 0; i < sizeof(stickyWhitelist) / sizeof(stickyWhitelist[0]); i++) {
        if (stickyWhitelist[i] == c) {
            return true;
        }
    }
    return false;
}
