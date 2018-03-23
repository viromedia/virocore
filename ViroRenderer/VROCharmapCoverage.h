//
//  VROCharmapCoverage.h
//  ViroKit
//
//  Created by Raj Advani on 3/21/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

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
