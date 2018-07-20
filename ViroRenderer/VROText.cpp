//
//  VROText.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROText.h"
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

static const int kVerticesPerGlyph = 6;

std::shared_ptr<VROText> VROText::createText(std::wstring text,
                                             std::string typefaceNames,
                                             int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                             VROVector4f color, float width, float height,
                                             VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                             VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines,
                                             std::shared_ptr<VRODriver> driver) {
    
    std::shared_ptr<VROText> model = std::make_shared<VROText>(text, typefaceNames, fontSize, fontStyle, fontWeight,
                                                               color, width, height, horizontalAlignment,
                                                               verticalAlignment, lineBreakMode, clipMode, maxLines,
                                                               driver);
    model->update();
    return model;
}

std::shared_ptr<VROText> VROText::createSingleLineText(std::wstring text,
                                                       std::string typefaceNames,
                                                       int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                       VROVector4f color,
                                                       float width, VROTextHorizontalAlignment alignment, VROTextClipMode clipMode,
                                                       std::shared_ptr<VRODriver> driver) {
    return createText(text, typefaceNames, fontSize, fontStyle, fontWeight, color, width,
                      std::numeric_limits<float>::max(), alignment, VROTextVerticalAlignment::Center,
                      VROLineBreakMode::None, clipMode, 0, driver);
}

std::shared_ptr<VROText> VROText::createSingleLineText(std::wstring text,
                                                       std::string typefaceNames,
                                                       int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                       VROVector4f color, std::shared_ptr<VRODriver> driver) {
    return createSingleLineText(text, typefaceNames, fontSize, fontStyle, fontWeight, color,
                                std::numeric_limits<float>::max(), VROTextHorizontalAlignment::Center,
                                VROTextClipMode::None, driver);
}

VROVector3f VROText::getTextSize(std::wstring text,
                                 std::shared_ptr<VROTypefaceCollection> typefaces,
                                 float maxWidth, float maxHeight, VROLineBreakMode lineBreakMode,
                                 VROTextClipMode clipMode, int maxLines) {
    
    VROVector3f size;
    std::map<uint32_t, std::shared_ptr<VROGlyph>> glyphMap;
    
    // Split the text into runs, eached mapped to a typeface
    std::vector<VROFontRun> fontRuns = typefaces->computeRuns(text);
    
    // Always add the space ' ' to the glyphMap (used by justification)
    std::shared_ptr<VROTypeface> &firstTypeface = fontRuns.front().typeface;
    std::wstring space = L" ";
    uint32_t spaceCode = *space.begin();
    glyphMap[spaceCode] = firstTypeface->getGlyph(spaceCode, 0, false);
    
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
                std::shared_ptr<VROGlyph> glyph = typeface->getGlyph(codePoint, 0, false);
                glyphMap[codePoint] = glyph;
            }
        }
    }
    
    std::vector<VROTextLine> lines;
    switch (lineBreakMode) {
        case VROLineBreakMode::WordWrap:
            lines = VROTextFormatter::wrapByWords(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::CharWrap:
            lines = VROTextFormatter::wrapByChars(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::None:
            lines = VROTextFormatter::wrapByNewlines(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::Justify:
            lines = VROTextFormatter::justify(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        default:
            pabort("Invalid linebreak mode found for VROText");
            break;
    }
    
    float lineHeight = maxLineHeight * kTextPointToWorldScale;
    size.y = lines.size() * lineHeight;
    
    std::vector<VROShapeVertexLayout> var;
    for (VROTextLine &textLine : lines) {
        std::wstring &line = textLine.line;
        
        float lineWidth = 0;
        for (std::wstring::const_iterator c = line.begin(); c != line.end(); ++c) {
            uint32_t charCode = *c;
            std::shared_ptr<VROGlyph> &glyph = glyphMap[charCode];
            
            lineWidth += glyph->getAdvance() * kTextPointToWorldScale;
        }
        
        size.x = std::max(size.x, lineWidth);
    }
    
    return size;
}

VROText::VROText(std::wstring text,
                 std::string typefaceNames,
                 int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                 VROVector4f color,
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
    _width(width),
    _height(height),
    _horizontalAlignment(horizontalAlignment),
    _verticalAlignment(verticalAlignment),
    _lineBreakMode(lineBreakMode),
    _clipMode(clipMode),
    _maxLines(maxLines),
    _driver(driver) {

}

void VROText::update() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (!driver) {
        return;
    }

    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;

    _typefaceCollection = driver->newTypefaceCollection(_typefaceNames, _size, _fontStyle, _fontWeight);

    float realizedWidth, realizedHeight;
    buildText(_text, _typefaceCollection, _color, _width, _height, _horizontalAlignment, _verticalAlignment,
              _lineBreakMode, _clipMode, _maxLines, sources, elements, materials,
              &realizedWidth, &realizedHeight, driver);

    _realizedWidth = realizedWidth;
    _realizedHeight = realizedHeight;

    setSources(sources);
    setElements(elements);
    setMaterials(materials);
    updateBoundingBox();
}

void VROText::setText(std::wstring text) {
    _text = text;
    update();
}

void VROText::setTypefaces(std::string typefaceNames, int size, VROFontStyle style, VROFontWeight weight) {
    _typefaceNames = typefaceNames;
    _size = size;
    _fontStyle = style;
    _fontWeight = weight;
    update();
}

void VROText::setColor(VROVector4f color) {
    _color = color;
    update();
}

void VROText::setWidth(float width) {
    _width = width;
    update();
}

void VROText::setHeight(float height) {
    _height = height;
    update();
}

void VROText::setHorizontalAlignment(VROTextHorizontalAlignment horizontalAlignment) {
    _horizontalAlignment = horizontalAlignment;
    update();
}

void VROText::setVerticalAlignment(VROTextVerticalAlignment verticalAlignment) {
    _verticalAlignment = verticalAlignment;
    update();
}

void VROText::setLineBreakMode(VROLineBreakMode lineBreakMode) {
    _lineBreakMode = lineBreakMode;
    update();
}

void VROText::setClipMode(VROTextClipMode clipMode) {
    _clipMode = clipMode;
    update();
}

void VROText::setMaxLines(int maxLines) {
    _maxLines = maxLines;
    update();
}

void VROText::buildText(std::wstring &text,
                        std::shared_ptr<VROTypefaceCollection> &typefaces,
                        VROVector4f color,
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
     Create a glyph, material, and vector of indices for each character
     in the text string. Characters that share the same atlas will use the
     same material.
     */
    std::map<uint32_t, std::shared_ptr<VROGlyph>> glyphMap;
    std::map<std::shared_ptr<VROGlyphAtlas>, std::pair<std::shared_ptr<VROMaterial>, std::vector<int>>> materialMap;
    
    // Split the text into runs, eached mapped to a typeface
    std::vector<VROFontRun> fontRuns = typefaces->computeRuns(text);
    
    // Always add the space ' ' to the glyphMap (used by justification)
    {
        std::shared_ptr<VROTypeface> &firstTypeface = fontRuns.front().typeface;
        std::wstring space = L" ";
        uint32_t spaceCode = *space.begin();
        
        std::shared_ptr<VROGlyph> whitespaceGlyph = firstTypeface->getGlyph(spaceCode, 0, true);
        
        std::shared_ptr<VROMaterial> whitespaceMaterial = std::make_shared<VROMaterial>();
        whitespaceMaterial->setNeedsToneMapping(false);
        whitespaceMaterial->getDiffuse().setColor(color);
        whitespaceMaterial->getDiffuse().setTexture(whitespaceGlyph->getTexture());
        
        char whitespaceName[2] = { (char)spaceCode, 0 };
        whitespaceMaterial->setName(whitespaceName);
        
        std::vector<int> indices;
        materialMap[whitespaceGlyph->getAtlas()] = { whitespaceMaterial, indices };
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
                std::shared_ptr<VROGlyph> glyph = typeface->getGlyph(codePoint, 0, true);
                
                const std::shared_ptr<VROGlyphAtlas> atlas = glyph->getAtlas();
                auto materialAndIndices = materialMap.find(atlas);
                if (materialAndIndices == materialMap.end()) {
                    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
                    material->setNeedsToneMapping(false);
                    material->getDiffuse().setColor(color);
  
                    std::vector<int> indices;
                    materialMap[atlas] = { material, indices };
                }
                glyphMap[codePoint] = glyph;
            }
        }
    }
    
    /*
     Now that all the glyphs are loaded for this text, refresh the atlas textures,
     then assign the texture to each material.
     */
    for (VROFontRun &fontRun : fontRuns) {
        std::shared_ptr<VROTypeface> &typeface = fontRun.typeface;
        typeface->refreshGlyphAtlases(driver);
    }
    for (auto &kv : glyphMap) {
        std::shared_ptr<VROGlyph> glyph = kv.second;
        std::shared_ptr<VROMaterial> material = materialMap[glyph->getAtlas()].first;
        material->getDiffuse().setTexture(glyph->getTexture());
    }
    
    std::vector<VROShapeVertexLayout> var;

    VROTextFormatter::formatAndBuild(text, width, height, maxLines, maxLineHeight, horizontalAlignment, verticalAlignment, lineBreakMode, clipMode, glyphMap, outRealizedWidth, outRealizedHeight,
                                     [&var, &materialMap] (std::shared_ptr<VROGlyph> &glyph, float x, float y) {
                                         buildChar(glyph, x, y, var, materialMap[glyph->getAtlas()].second);
                                     });
    buildGeometry(var, materialMap, sources, elements, materials);
}

void VROText::buildChar(std::shared_ptr<VROGlyph> &glyph,
                        float x, float y,
                        std::vector<VROShapeVertexLayout> &var,
                        std::vector<int> &indices) {
    
    int index = (int)var.size();
    
    x += glyph->getBearing().x * kTextPointToWorldScale;
    y += (glyph->getBearing().y - glyph->getSize().y) * kTextPointToWorldScale;
    
    float w = glyph->getSize().x * kTextPointToWorldScale;
    float h = glyph->getSize().y * kTextPointToWorldScale;
    
    float minU = glyph->getMinU();
    float maxU = glyph->getMaxU();
    float minV = glyph->getMinV();
    float maxV = glyph->getMaxV();
    
    var.push_back({x,     y + h, 0, minU, minV, 0, 0, 1});
    var.push_back({x,     y,     0, minU, maxV, 0, 0, 1});
    var.push_back({x + w, y,     0, maxU, maxV, 0, 0, 1});
    var.push_back({x,     y + h, 0, minU, minV, 0, 0, 1});
    var.push_back({x + w, y,     0, maxU, maxV, 0, 0, 1});
    var.push_back({x + w, y + h, 0, maxU, minV, 0, 0, 1});
    
    for (int i = 0; i < kVerticesPerGlyph; i++) {
        indices.push_back(index + i);
    }
}

void VROText::buildGeometry(std::vector<VROShapeVertexLayout> &var,
                            std::map<std::shared_ptr<VROGlyphAtlas>, std::pair<std::shared_ptr<VROMaterial>, std::vector<int>>> &materialMap,
                            std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                            std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                            std::vector<std::shared_ptr<VROMaterial>> &materials) {
    
    int numVertices = (int) var.size();
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>(var.data(), var.size() * sizeof(VROShapeVertexLayout));
    
    std::vector<std::shared_ptr<VROGeometrySource>> genSources = VROShapeUtilBuildGeometrySources(vertexData, numVertices);
    for (std::shared_ptr<VROGeometrySource> source : genSources) {
        sources.push_back(source);
    }
    
    for (auto &kv : materialMap) {
        std::shared_ptr<VROMaterial> &material = kv.second.first;
        std::vector<int> &indices = kv.second.second;
        
        std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices.data(), sizeof(int) * indices.size());
        std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                           VROGeometryPrimitiveType::Triangle,
                                                                                           indices.size() / 3,
                                                                                           sizeof(int));
        elements.push_back(element);
        materials.push_back(material);
    }
}

VROText::~VROText() {
    
}
