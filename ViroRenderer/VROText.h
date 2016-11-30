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
#include "VROShapeUtils.h"

class VROMaterial;
class VROTexture;

enum class VROTextHorizontalAlignment {
    Left,
    Right,
    Center,
    Justified
};

enum class VROTextVerticalAlignment {
    Top,
    Bottom,
    Center
};

enum class VROLineBreakMode {
    WordWrap,
    CharWrap,
    Clip,
    TruncateTail
};

class VROTextLayout {
    float width;
    float height;
    VROTextHorizontalAlignment horizontalAlignment;
    VROTextVerticalAlignment verticalAlignment;
};

class VROText : public VROGeometry {
    
public:
    
    static std::shared_ptr<VROText> createText(std::string text, std::shared_ptr<VROTypeface> typeface, float width, float height,
                                               VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                               VROLineBreakMode lineBreakMode, int maxLines = 0);
    
    static VROVector3f getTextSize(std::string text, std::shared_ptr<VROTypeface> typeface,
                                   float maxWidth, VROLineBreakMode lineBreakMode, int maxLines = 0);
    
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
                              float width,
                              float height,
                              VROTextHorizontalAlignment horizontalAlignment,
                              VROTextVerticalAlignment verticalAlignment,
                              VROLineBreakMode lineBreakMode,
                              int maxLines,
                              std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                              std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                              std::vector<std::shared_ptr<VROMaterial>> &materials,
                              float *outRealizedWidth, float *outRealizedHeight);
    
    static void buildChar(std::vector<VROShapeVertexLayout> &var,
                          char c, float x, float y, float w, float h,
                          float minU, float maxU, float minV, float maxV,
                          std::shared_ptr<VROTexture> texture,
                          std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                          std::vector<std::shared_ptr<VROMaterial>> &materials);
    
    float _width, _height;
    
};

#endif /* VROText_h */
