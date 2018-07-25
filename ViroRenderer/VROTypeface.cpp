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
#include "VROCharmapCoverage.h"
#include "VROFontUtil.h"
#include "VROStringUtil.h"
#include "VROGlyphAtlas.h"
#include "VROGlyph.h"

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
    FT_FaceRec_ *face = loadFTFace();
    computeCoverage(face);
}

bool VROTypeface::hasCharacter(uint32_t codepoint, uint32_t variationSelector) const {
    if (variationSelector == 0) {
        return _coverage.get(codepoint);
    }
    
    // If we're looking for a variation but we don't have a variation cmap table
    // (table 14), then return false
    if (_variationCoverage.empty()) {
        return false;
    }
    
    const uint16_t vsIndex = VROFontUtil::getVsIndex(variationSelector);
    if (vsIndex >= _variationCoverage.size()) {
        // Note that kInvalidVSIndex also reaches here
        return false;
    }
    
    const std::unique_ptr<VROSparseBitSet> &bitset = _variationCoverage[vsIndex];
    if (bitset.get() == nullptr) {
        return false;
    }
    return bitset->get(codepoint);
}

void VROTypeface::computeCoverage(FT_FaceRec_* face) {
    FT_ULong cmapTag = FT_MAKE_TAG('c', 'm', 'a', 'p');
    FT_ULong cmapLength = 0;
    FT_Error error = FT_Load_Sfnt_Table(face, cmapTag, 0, NULL, &cmapLength );
    if (error) {
        pinfo("Cmap table does not exist, no coverage computed");
        return;
    }
    
    VROByteBuffer buffer(cmapLength);
    error = FT_Load_Sfnt_Table(face, cmapTag, 0, (FT_Byte *)buffer.getData(), &cmapLength);
    if (error) {
        pinfo("Failed to load 'meta' table");
    }
    _coverage = VROCharmapCoverage::getCoverage((uint8_t *) buffer.getData(), buffer.capacity(), &_variationCoverage);
}

std::pair<std::string, std::string> VROTypeface::getLanguages(FT_FaceRec_* face) {
    std::string slng, dlng;
    FT_ULong metaTag = FT_MAKE_TAG('m', 'e', 't', 'a');
    FT_ULong metaLength = 0;
    FT_Error error = FT_Load_Sfnt_Table(face, metaTag, 0, NULL, &metaLength );
    if (error) {
        pinfo("Meta table does not exist");
        std::make_pair(dlng, slng);
    }
    
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
    return std::make_pair(dlng, slng);
}

std::shared_ptr<VROGlyph> VROTypeface::getGlyph(uint32_t codePoint, uint32_t variantSelector,
                                                uint32_t outlineWidth, VROGlyphRenderMode renderMode) {
    std::string key = VROStringUtil::toString(codePoint) + "_V" +
                      VROStringUtil::toString(variantSelector) + "_S" +
                      VROStringUtil::toString(outlineWidth);
    
    if (renderMode == VROGlyphRenderMode::Bitmap || renderMode == VROGlyphRenderMode::None) {
        auto kv = _bitmapGlyphCache.find(key);
        if (kv != _bitmapGlyphCache.end()) {
            return kv->second;
        }
    } else if (renderMode == VROGlyphRenderMode::Vector || renderMode == VROGlyphRenderMode::None) {
        auto kv = _vectorGlyphCache.find(key);
        if (kv != _vectorGlyphCache.end()) {
            return kv->second;
        }
    }
    
    std::shared_ptr<VROGlyph> glyph = loadGlyph(codePoint, variantSelector, outlineWidth, renderMode);
    if (renderMode == VROGlyphRenderMode::Bitmap) {
        _bitmapGlyphCache.insert(std::make_pair(key, glyph));
    } else if (renderMode == VROGlyphRenderMode::Vector) {
        _vectorGlyphCache.insert(std::make_pair(key, glyph));
    }
    return glyph;
}

void VROTypeface::preloadGlyphs(std::string chars) {
    for (std::string::const_iterator c = chars.begin(); c != chars.end(); ++c) {
        getGlyph(*c, 0, 0, VROGlyphRenderMode::Bitmap);
    }
}

void VROTypeface::refreshGlyphAtlases(std::shared_ptr<VRODriver> driver) {
    for (std::shared_ptr<VROGlyphAtlas> atlas : _glyphAtlases) {
        atlas->refreshTexture(driver);
    }
    for (auto &kv : _outlineAtlases) {
        for (std::shared_ptr<VROGlyphAtlas> atlas : kv.second) {
            atlas->refreshTexture(driver);
        }
    }
}
