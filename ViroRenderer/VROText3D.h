//
//  VROText3D.hpp
//  ViroKit
//
//  Created by Raj Advani on 7/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROText3D_h
#define VROText3D_h

#include "VROText.h"

class VROText3D : public VROGeometry {
public:
    
    /*
     Create a text object for displaying the given string with the typefaces in the
     given collection, constrained to the bounds defined by the provided width and height,
     and aligned according to the given alignment parameters and linebreak mode.
     
     The clip mode determines whether the text is clipped to the given bounds.
     
     The maxLines parameter, if set, caps the number of lines; when zero, there is no
     limit to the number of lines generated.
     
     The typeface to use for each character will be chosen from among the typeface collection.
     */
    static std::shared_ptr<VROText3D> createText(std::wstring text,
                                                 std::string typefaceNames,
                                                 int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                 VROVector4f color, float extrusion,
                                                 float width, float height,
                                                 VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                                 VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines,
                                                 std::shared_ptr<VRODriver> driver);
    
    /*
     Helper method to create a single-line text in a horizontal box of the given width.
     The box is centered at the parent node's position, and the text is aligned within the
     box according to the given alignment.
     */
    static std::shared_ptr<VROText3D> createSingleLineText(std::wstring text,
                                                           std::string typefaceNames,
                                                           int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                           VROVector4f color, float extrusion,
                                                           float width, VROTextHorizontalAlignment alignment, VROTextClipMode clipMode,
                                                           std::shared_ptr<VRODriver> driver);
    
    /*
     Helper method to create a centered single-line text. The text will be centered (vertically
     and horizontally) about the parent node's position.
     */
    static std::shared_ptr<VROText3D> createSingleLineText(std::wstring text,
                                                           std::string typefaceNames,
                                                           int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                           VROVector4f color, float extrusion,
                                                           std::shared_ptr<VRODriver> driver);
    
    /*
     Return the width and height of a text object displaying the given string, with the
     given typeface collection.
     */
    static VROVector3f getTextSize(std::wstring text,
                                   std::shared_ptr<VROTypefaceCollection> typefaces,
                                   float maxWidth, float maxHeight, VROLineBreakMode lineBreakMode,
                                   VROTextClipMode clipMode, int maxLines);
    
    /*
     Standard constructor. Update() must be invoked from the rendering thread if this constructor
     is used.
     */
    VROText3D(std::wstring text,
              std::string typefaceNames,
              int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
              VROVector4f color, float extrusion,
              float width, float height,
              VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
              VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines,
              std::shared_ptr<VRODriver> driver);
    virtual ~VROText3D();
    
    /*
     Initialize the VROText3D if any fields have changed. This must be invoked on the rendering
     thread because it creates glyphs.
     */
    void update();
    
    /*
     Return the typeface collection used by this text. This is only valid after update() has
     been called.
     */
    std::shared_ptr<VROTypefaceCollection> getTypefaceCollection() {
        return _typefaceCollection;
    }
    
    /*
     Get the width and height of the text. This is not the width and height of the
     bounds used when the text was created, but the width and height of the actual
     text.
     */
    float getRealizedWidth() const {
        return _realizedWidth;
    }
    float getRealizedHeight() const {
        return _realizedHeight;
    }
    
    void setText(std::wstring text);
    void setTypefaces(std::string typefaces, int size, VROFontStyle style, VROFontWeight weight);
    void setColor(VROVector4f color);
    void setWidth(float width);
    void setHeight(float height);
    void setHorizontalAlignment(VROTextHorizontalAlignment horizontalAlignment);
    void setVerticalAlignment(VROTextVerticalAlignment verticalAlignment);
    void setLineBreakMode(VROLineBreakMode lineBreakMode);
    void setClipMode(VROTextClipMode clipMode);
    void setMaxLines(int maxLines);
    
private:
    
    std::wstring _text;
    std::shared_ptr<VROTypefaceCollection> _typefaceCollection;
    std::string _typefaceNames;
    int _size;
    VROFontStyle _fontStyle;
    VROFontWeight _fontWeight;
    VROVector4f _color;
    float _extrusion;
    float _width, _height;
    VROTextHorizontalAlignment _horizontalAlignment;
    VROTextVerticalAlignment _verticalAlignment;
    VROLineBreakMode _lineBreakMode;
    VROTextClipMode _clipMode;
    int _maxLines;
    std::weak_ptr<VRODriver> _driver;
    
    std::atomic<float> _realizedWidth, _realizedHeight;
    
    VROText3D(std::vector<std::shared_ptr<VROGeometrySource>> sources,
            std::vector<std::shared_ptr<VROGeometryElement>> elements,
            float width, float height) :
        VROGeometry(sources, elements),
        _width(width),
        _height(height)
    {}
    
    static void buildText(std::wstring &text,
                          std::shared_ptr<VROTypefaceCollection> &typefaces,
                          VROVector4f color,
                          float extrusion,
                          float width,
                          float height,
                          VROTextHorizontalAlignment horizontalAlignment,
                          VROTextVerticalAlignment verticalAlignment,
                          VROLineBreakMode lineBreakMode,
                          VROTextClipMode clipMode,
                          int maxLines,
                          std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                          std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                          std::vector<std::shared_ptr<VROMaterial>> &materials,
                          float *outRealizedWidth, float *outRealizedHeight,
                          std::shared_ptr<VRODriver> driver);
    
    /*
     Build a standard Viro geometry from the given vertex array and indices.
     */
    static void buildGeometry(std::vector<VROShapeVertexLayout> &var,
                              std::vector<int> &indices,
                              std::shared_ptr<VROMaterial> material,
                              std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                              std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                              std::vector<std::shared_ptr<VROMaterial>> &materials);
    
    /*
     Write the geometry for the given glyph (at the given position) into the
     provided vertex array, and write the associated indices into the indices
     array as well.
     */
    static void buildChar(std::shared_ptr<VROGlyph> &glyph,
                          float x, float y, float extrusion,
                          std::vector<VROShapeVertexLayout> &var,
                          std::vector<int> &indices);
};

#endif /* VROText3D_h */
