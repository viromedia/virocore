//
//  VROTypeface.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/13/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTypeface.h"
#include <ft2build.h>
#include "VROByteBuffer.h"
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

template <typename T>
T swap_endian(T u) {
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;
    
    source.u = u;
    for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1];
    
    return dest.u;
}

VROTypeface::VROTypeface(std::string name, int size, VROFontStyle style, VROFontWeight weight) :
    _name(name),
    _size(size),
    _style(style),
    _weight(weight) {
    
    ALLOCATION_TRACKER_ADD(Typefaces, 1);
}

VROTypeface::~VROTypeface() {
    ALLOCATION_TRACKER_SUB(Typefaces, 1);
}

void VROTypeface::loadFace() {
    loadFace(_name, _size);
}

std::pair<std::string, std::string> VROTypeface::getLanguages(FT_FaceRec_* face) {
    std::string slng, dlng;
    FT_ULong metaTag = FT_MAKE_TAG('m', 'e', 't', 'a');
    
    FT_ULong  metaLength = 0;
    
    FT_Error error = FT_Load_Sfnt_Table(face, metaTag, 0, NULL, &metaLength );
    if (error) {
        pinfo("Meta table does not exist");
    }
    else {
        VROByteBuffer buffer(metaLength);
        error = FT_Load_Sfnt_Table(face, metaTag, 0, (FT_Byte *)buffer.getData(), &metaLength);
        if (error) {
            pinfo("Failed to load 'meta' table");
        }
        
        buffer.readInt(); // Version
        buffer.readInt(); // Flags
        buffer.readInt(); // Data offset
        
        int numDataMaps = swap_endian<uint32_t>(buffer.readInt());
        
        for (int i = 0; i < numDataMaps; i++) {
            int tag = swap_endian<uint32_t>(buffer.readInt());
            int mapOffset = swap_endian<uint32_t>(buffer.readInt());
            int mapLength = swap_endian<uint32_t>(buffer.readInt());
            
            size_t savedPos = buffer.getPosition();
            buffer.setPosition(mapOffset);
            
            char lng[mapLength + 1];
            buffer.copyBytes(lng, mapLength);
            lng[mapLength] = '\0';
            buffer.setPosition(savedPos);
            
            if (tag == 'dlng') {
                pinfo("Designed for languages: %s", lng);
                dlng = lng;
            } else if (tag == 'slng') {
                pinfo("Supports languages %s", lng);
                slng = lng;
            }
        }
    }
    return std::make_pair(dlng, slng);
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
