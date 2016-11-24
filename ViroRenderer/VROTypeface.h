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

class VROGlyph;

class VROTypeface {
    
public:
    
    VROTypeface(std::string name) :
        _name(name) {}
    virtual ~VROTypeface() {}
    
    std::string getName() const {
        return _name;
    }
    
    virtual std::unique_ptr<VROGlyph> loadGlyph(FT_ULong charCode) = 0;
    
private:
    
    std::string _name;
    
};

#endif /* VROTypeface_h */
