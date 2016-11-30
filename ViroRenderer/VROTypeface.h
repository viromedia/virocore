//
//  VROTypeface.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROTypeface_h
#define VROTypeface_h

#include <stdio.h>
#include <string>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "VROLog.h"

class VROGlyph;

class VROTypeface {
    
public:
    
    VROTypeface(std::string name, int size) :
        _name(name),
        _size(size) {
    
        if (FT_Init_FreeType(&_ft)) {
            pabort("Could not initialize freetype library");
        }
    }
    
    virtual ~VROTypeface() {
        FT_Done_Face(_face);
        FT_Done_FreeType(_ft);
    }
    
    std::string getName() const {
        return _name;
    }
    
    void loadFace() {
        _face = loadFace(_name, _size, _ft);
    }
    virtual std::unique_ptr<VROGlyph> loadGlyph(FT_ULong charCode) = 0;
    
    float getLineHeight() const {
        return _face->size->metrics.height >> 6;
    }
    float getMaxAdvance() const {
        return _face->size->metrics.max_advance >> 6;
    }
    
protected:
    
    virtual FT_Face loadFace(std::string name, int size, FT_Library ft) = 0;
    
    std::string _name;
    int _size;
    
    FT_Library _ft;
    FT_Face _face;
    
};

#endif /* VROTypeface_h */
