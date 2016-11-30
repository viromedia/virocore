//
//  VROText.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROText_h
#define VROText_h

#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "VROGeometry.h"

class VROMaterial;

class VROText : public VROGeometry {
    
public:
    
    static std::shared_ptr<VROText> createText(std::string text, std::string typefaceName, int pointSize, VRODriver &driver);
    virtual ~VROText();
    
    /*
     Get the width and height of the text.
     */
    float getWidth() const {
        return _width;
    }
    float getHeight() const {
        return _height;
    }
    
private:
    
    VROText(std::vector<std::shared_ptr<VROGeometrySource>> sources,
            std::vector<std::shared_ptr<VROGeometryElement>> elements,
            float width, float height) :
        VROGeometry(sources, elements),
        _width(width),
        _height(height)
    {}
    
    static void buildGeometry(std::string text,
                              std::shared_ptr<VROTypeface> typeface,
                              float scale,
                              VRODriver &driver,
                              std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                              std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                              std::vector<std::shared_ptr<VROMaterial>> &materials,
                              float *width, float *height);
    
    float _width, _height;
    
};

#endif /* VROText_h */
