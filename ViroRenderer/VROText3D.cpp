//
//  VROText3D.cpp
//  ViroKit
//
//  Created by Raj Advani on 7/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROText3D.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROMaterial.h"
#include "VROTypeface.h"
#include "VROTypefaceCollection.h"
#include "VROGlyph.h"
#include "VROLog.h"
#include <cstddef>
#include <limits>
#include "VROTextFormatter.h"
#include "VROStringUtil.h"
#include <ft2build.h>
#include FT_FREETYPE_H

std::shared_ptr<VROText3D> VROText3D::createText(std::wstring text,
                                                 std::string typefaceNames,
                                                 int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                 VROVector4f color, float extrusion,
                                                 float width, float height,
                                                 VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                                 VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines,
                                                 std::shared_ptr<VRODriver> driver) {
    
    std::shared_ptr<VROText3D> model = std::make_shared<VROText3D>(text, typefaceNames, fontSize, fontStyle, fontWeight,
                                                                   color, extrusion, width, height, horizontalAlignment,
                                                                   verticalAlignment, lineBreakMode, clipMode, maxLines,
                                                                   driver);
    model->update();
    return model;
}

std::shared_ptr<VROText3D> VROText3D::createSingleLineText(std::wstring text,
                                                       std::string typefaceNames,
                                                       int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                       VROVector4f color, float extrusion,
                                                       float width, VROTextHorizontalAlignment alignment, VROTextClipMode clipMode,
                                                       std::shared_ptr<VRODriver> driver) {
    return createText(text, typefaceNames, fontSize, fontStyle, fontWeight, color, extrusion, width,
                      std::numeric_limits<float>::max(), alignment, VROTextVerticalAlignment::Center,
                      VROLineBreakMode::None, clipMode, 0, driver);
}

std::shared_ptr<VROText3D> VROText3D::createSingleLineText(std::wstring text,
                                                       std::string typefaceNames,
                                                       int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                       VROVector4f color, float extrusion, std::shared_ptr<VRODriver> driver) {
    return createSingleLineText(text, typefaceNames, fontSize, fontStyle, fontWeight, color, extrusion,
                                std::numeric_limits<float>::max(), VROTextHorizontalAlignment::Center,
                                VROTextClipMode::None, driver);
}

VROVector3f VROText3D::getTextSize(std::wstring text,
                                   std::shared_ptr<VROTypefaceCollection> typefaces,
                                   float maxWidth, float maxHeight, VROLineBreakMode lineBreakMode,
                                   VROTextClipMode clipMode, int maxLines) {
    
    return VROText::getTextSize(text, typefaces, maxWidth, maxHeight, lineBreakMode, clipMode, maxLines);
}

VROText3D::VROText3D(std::wstring text,
                     std::string typefaceNames,
                     int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                     VROVector4f color, float extrusion,
                     float width, float height,
                     VROTextHorizontalAlignment horizontalAlignment,
                     VROTextVerticalAlignment verticalAlignment,
                     VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines,
                     std::shared_ptr<VRODriver> driver) :
_text(text),
_typefaceNames(typefaceNames),
_size(fontSize),
_fontStyle(fontStyle),
_fontWeight(fontWeight),
_color(color),
_extrusion(extrusion),
_width(width),
_height(height),
_horizontalAlignment(horizontalAlignment),
_verticalAlignment(verticalAlignment),
_lineBreakMode(lineBreakMode),
_clipMode(clipMode),
_maxLines(maxLines),
_driver(driver) {
    
}

VROText3D::~VROText3D() {
    
}

void VROText3D::update() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (!driver) {
        return;
    }
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;
    
    _typefaceCollection = driver->newTypefaceCollection(_typefaceNames, _size, _fontStyle, _fontWeight);
    
    float realizedWidth, realizedHeight;
    buildText(_text, _typefaceCollection, _color, _extrusion, _width, _height, _horizontalAlignment, _verticalAlignment,
              _lineBreakMode, _clipMode, _maxLines, sources, elements, materials,
              &realizedWidth, &realizedHeight, driver);
    
    _realizedWidth = realizedWidth;
    _realizedHeight = realizedHeight;
    
    setSources(sources);
    setElements(elements);
    setMaterials(materials);
    updateBoundingBox();
}

void VROText3D::setText(std::wstring text) {
    _text = text;
    update();
}

void VROText3D::setTypefaces(std::string typefaceNames, int size, VROFontStyle style, VROFontWeight weight) {
    _typefaceNames = typefaceNames;
    _size = size;
    _fontStyle = style;
    _fontWeight = weight;
    update();
}

void VROText3D::setColor(VROVector4f color) {
    _color = color;
    update();
}

void VROText3D::setWidth(float width) {
    _width = width;
    update();
}

void VROText3D::setHeight(float height) {
    _height = height;
    update();
}

void VROText3D::setHorizontalAlignment(VROTextHorizontalAlignment horizontalAlignment) {
    _horizontalAlignment = horizontalAlignment;
    update();
}

void VROText3D::setVerticalAlignment(VROTextVerticalAlignment verticalAlignment) {
    _verticalAlignment = verticalAlignment;
    update();
}

void VROText3D::setLineBreakMode(VROLineBreakMode lineBreakMode) {
    _lineBreakMode = lineBreakMode;
    update();
}

void VROText3D::setClipMode(VROTextClipMode clipMode) {
    _clipMode = clipMode;
    update();
}

void VROText3D::setMaxLines(int maxLines) {
    _maxLines = maxLines;
    update();
}

void VROText3D::buildText(std::wstring &text,
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
                          std::shared_ptr<VRODriver> driver) {
    if (text.size() == 0) {
        *outRealizedWidth = 0;
        *outRealizedHeight = 0;
        return;
    }
    
    /*
     Create a glyph for each character in the text string. All characters in vectorized
     text use the same material.
     */
    std::map<uint32_t, std::shared_ptr<VROGlyph>> glyphMap;
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->getDiffuse().setColor(color);
    material->setCullMode(VROCullMode::None);
    
    // Split the text into runs, eached mapped to a typeface
    std::vector<VROFontRun> fontRuns = typefaces->computeRuns(text);
    
    // Always add the space ' ' to the glyphMap (used by justification)
    {
        std::shared_ptr<VROTypeface> &firstTypeface = fontRuns.front().typeface;
        std::wstring space = L" ";
        uint32_t spaceCode = *space.begin();
        
        std::shared_ptr<VROGlyph> whitespaceGlyph = firstTypeface->getGlyph(spaceCode, 0, VROGlyphRenderMode::Vector);
        glyphMap[spaceCode] = whitespaceGlyph;
    }
    
    // Now add all the remaining glyphs. For line height we will use
    // the maximum found in any run.
    float maxLineHeight = 0;
    
    for (VROFontRun &fontRun : fontRuns) {
        std::shared_ptr<VROTypeface> &typeface = fontRun.typeface;
        if (typeface->getLineHeight() > maxLineHeight) {
            maxLineHeight = typeface->getLineHeight();
        }
        
        for (int i = fontRun.start; i < fontRun.end; i++) {
            uint32_t codePoint = text.at(i);
            if (glyphMap.find(codePoint) == glyphMap.end()) {
                std::shared_ptr<VROGlyph> glyph = typeface->getGlyph(codePoint, 0, VROGlyphRenderMode::Vector);
                glyphMap[codePoint] = glyph;
            }
        }
    }
    
    std::vector<VROShapeVertexLayout> var;
    std::vector<int> indices;
    
    VROTextFormatter::formatAndBuild(text, width, height, maxLines, maxLineHeight, horizontalAlignment, verticalAlignment, lineBreakMode, clipMode, glyphMap, outRealizedWidth, outRealizedHeight,
                                     [&var, &indices, extrusion] (std::shared_ptr<VROGlyph> &glyph, float x, float y) {
                                         buildChar(glyph, x, y, extrusion, var, indices);
                                     });
    buildGeometry(var, indices, material, sources, elements, materials);
}

void VROText3D::buildChar(std::shared_ptr<VROGlyph> &glyph,
                          float x, float y, float extrusion,
                          std::vector<VROShapeVertexLayout> &var,
                          std::vector<int> &indices) {
    
    const std::vector<VROTriangle> &triangles = glyph->getTriangles();
    //x += glyph->getBearing().x * kTextPointToWorldScale;
    //y += (glyph->getBearing().y - glyph->getSize().y) * kTextPointToWorldScale;
    
    for (const VROTriangle &triangle : triangles) {
        var.push_back({ x + triangle.getA().x * kTextPointToWorldScale,
                        y + triangle.getA().y * kTextPointToWorldScale,
                            triangle.getA().z * kTextPointToWorldScale * extrusion, 0, 0, 0, 0, 1});
        var.push_back({ x + triangle.getB().x * kTextPointToWorldScale,
                        y + triangle.getB().y * kTextPointToWorldScale,
                            triangle.getB().z * kTextPointToWorldScale * extrusion, 0, 0, 0, 0, 1});
        var.push_back({ x + triangle.getC().x * kTextPointToWorldScale,
                        y + triangle.getC().y * kTextPointToWorldScale,
                            triangle.getC().z * kTextPointToWorldScale * extrusion, 0, 0, 0, 0, 1});
        for (int i = 0; i < 3; i++) {
            indices.push_back((int) indices.size());
        }
    }
    
    
}

void VROText3D::buildGeometry(std::vector<VROShapeVertexLayout> &var,
                              std::vector<int> &indices,
                              std::shared_ptr<VROMaterial> material,
                              std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                              std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                              std::vector<std::shared_ptr<VROMaterial>> &materials) {
    
    int numVertices = (int) var.size();
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>(var.data(), var.size() * sizeof(VROShapeVertexLayout));
    
    std::vector<std::shared_ptr<VROGeometrySource>> genSources = VROShapeUtilBuildGeometrySources(vertexData, numVertices);
    for (std::shared_ptr<VROGeometrySource> source : genSources) {
        sources.push_back(source);
    }
    
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices.data(), sizeof(int) * indices.size());
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::Triangle,
                                                                                       indices.size() / 3,
                                                                                       sizeof(int));
    elements.push_back(element);
    materials.push_back(material);
}
