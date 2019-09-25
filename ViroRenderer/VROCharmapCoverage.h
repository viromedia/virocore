//
//  VROCharmapCoverage.h
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

#ifndef VROCharmapCoverage_h
#define VROCharmapCoverage_h

#include "VROSparseBitSet.h"
#include <memory>
#include <vector>

/*
 Reads a TTF/OTF CMAP table and parses it into a bit-set that can be used to quickly
 determine what glyphs a typeface supports.
 */
class VROCharmapCoverage {
public:
    
    /*
     Return the format 4 or format 12 coverage given raw cmap table data. In the given
     output talbe, store format 14 coverage (for variation sequences). For detail on
     the cmap table's formats, see the following link:
     
     https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
     */
    static VROSparseBitSet getCoverage(const uint8_t *cmap_data, size_t cmap_size,
                                       std::vector<std::unique_ptr<VROSparseBitSet>> *out);
};

#endif  // VROCharmapCoverage_h
