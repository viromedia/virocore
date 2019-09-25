//
//  VROGlyph.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

#ifndef VROGlyph_h
#define VROGlyph_h

#include <map>
#include <memory>
#include <vector>
#include "VROVector3f.h"
#include "VROTriangle.h"
#include "VROAllocationTracker.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class VRODriver;
class VROTexture;
class VROGlyphAtlas;
class VROAtlasLocation;
class VROGlyphTriangle;

enum class VROGlyphRenderMode {
    None,
    Bitmap,
    Vector,
};

/*
 Data used to render glyphs as bitmaps.
 */
class VROGlyphBitmap {
public:
    /*
     Atlas on which the glyph is rendered.
     */
    std::shared_ptr<VROGlyphAtlas> atlas;
    
    /*
     Size of the glyph.
     */
    VROVector3f size;
    
    /*
     Offset from the baseline to the top-left of the glyph.
     */
    VROVector3f bearing;

    /*
     The min and max U and V values for the texture. These
     indicate what section of the texture corresponds to
     this character.
     */
    float minU, minV;
    float maxU, maxV;
};

class VROGlyph {
public:
    
    VROGlyph() {
        ALLOCATION_TRACKER_ADD(Glyphs, 1);
    }
    virtual ~VROGlyph() {
        ALLOCATION_TRACKER_SUB(Glyphs, 1);
    }
    
    /*
     Load the glyph identified by the given FT_Face. These methods may be called
     successively to load different pieces of the FT_Face into this glyph.
     
     The first method is used to only load the metrics of the glyph for sizing
     operations. Only getAdvance() will be available after invoking this method.
     
     The loadBitmap method will write the glyph's bitmap to the
     last VROGlyphAtlas provided in the given vector. If the glyph does not
     fit in said atlas, then a new VROGlyphAtlas will be created and pushed
     to the back of the vector. After invoking this method, the bitmap data
     will be available by invoking getBitmap(0).
     
     The third method loads the bitmap of an outline into the glyph. The bitmap
     will have the provided outline width, and will similarly write to the provided
     atlas vector. The loaded bitmap data will be available by calling
     getBitmap(outlineWidth).
     
     Finally, the loadVector method craetes the contours of the glyph in 3D,
     triangulates them, and stores the results, which are accessible via
     getTriangles(). This method is used for 3D text rendering.
     
     If the variant selector is 0, then we assume this is not a variation
     sequence.
     */
    virtual bool loadMetrics(FT_Face face, uint32_t charCode, uint32_t variantSelector) = 0;
    virtual bool loadBitmap(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                            std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                            std::shared_ptr<VRODriver> driver) = 0;
    virtual bool loadOutlineBitmap(FT_Library library, FT_Face face, uint32_t charCode, uint32_t variantSelector,
                                   uint32_t outlineWidth,
                                   std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                                   std::shared_ptr<VRODriver> driver) = 0;
    virtual bool loadVector(FT_Face face, uint32_t charCode, uint32_t variantSelector) = 0;
    
#pragma mark - Bitmap Fonts
    
    /*
     For glyphs rendered to textures, these methods provide the atlas and other
     metadata required for rendering.
     */
    const VROGlyphBitmap &getBitmap(int outlineWidth) const {
        return _bitmaps.find(outlineWidth)->second;
    }
    
#pragma mark - Vector Fonts
    
    const std::vector<VROGlyphTriangle> &getTriangles() const {
        return _triangles;
    }
    
#pragma mark - All Fonts
    
    long getAdvance() const {
        return _advance;
    }
    
protected:
    
    /*
     Atlas on which the glyph is rendered, for different outline widths.
     Width 0 corresponds to no outline.
     */
    std::map<int, VROGlyphBitmap> _bitmaps;
    
    /*
     Offset to advance to the next glyph.
     */
    long _advance;
    
    /*
     For vectorized glphs, contains the triangles to render the
     glyph.
     */
    std::vector<VROGlyphTriangle> _triangles;
    
};

enum class VROGlyphTriangleType {
    Front,
    Back,
    Side
};

class VROGlyphTriangle {
public:
    
    VROGlyphTriangle(VROVector3f a, VROVector3f b, VROVector3f c, VROGlyphTriangleType type) :
        _a(a), _b(b), _c(c), _type(type) {}
    virtual ~VROGlyphTriangle() {}
    
    /*
     Get the points of the triangle.
     */
    VROVector3f getA() const { return _a; }
    VROVector3f getB() const { return _b; }
    VROVector3f getC() const { return _c; }
    VROGlyphTriangleType getType() const { return _type; }
    
private:
    
    VROVector3f _a;
    VROVector3f _b;
    VROVector3f _c;
    VROGlyphTriangleType _type;
};

#endif /* VROGlyph_h */
