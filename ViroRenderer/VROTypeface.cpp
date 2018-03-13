//
//  VROTypeface.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/13/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTypeface.h"
#include <ft2build.h>
#include FT_FREETYPE_H

VROTypeface::VROTypeface(std::string name, int size) :
    _name(name),
    _size(size) {
    
    ALLOCATION_TRACKER_ADD(Typefaces, 1);
}

VROTypeface::~VROTypeface() {
    ALLOCATION_TRACKER_SUB(Typefaces, 1);
}

void VROTypeface::loadFace() {
    loadFace(_name, _size);
}

std::shared_ptr<VROGlyph> VROTypeface::getGlyph(FT_ULong charCode, bool forRendering) {
    auto kv = _glyphCache.find(charCode);
    if (kv != _glyphCache.end()) {
        return kv->second;
    }
    
    std::shared_ptr<VROGlyph> glyph = loadGlyph(charCode, forRendering);
    if (forRendering) {
        _glyphCache.insert(std::make_pair(charCode, glyph));
    }
    return glyph;
}

void VROTypeface::preloadGlyphs(std::string chars) {
    for (std::string::const_iterator c = chars.begin(); c != chars.end(); ++c) {
        getGlyph(*c, true);
    }
}
