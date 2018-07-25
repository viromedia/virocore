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
#include "VROGlyphAtlas.h"
#include <ft2build.h>
#include FT_FREETYPE_H

static const int kVerticesPerGlyph = 6;

std::shared_ptr<VROText> VROText::createText(std::wstring text,
                                             std::string typefaceNames,
                                             int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                             VROVector4f color, float extrusion, float width, float height,
                                             VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                             VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines,
                                             std::shared_ptr<VRODriver> driver) {
    
    VROVector4f strokeColor = {1, 1, 1, 1};
    std::shared_ptr<VROText> model = std::make_shared<VROText>(text, typefaceNames, fontSize, fontStyle, fontWeight,
                                                               color, extrusion, VROTextOuterStroke::None, 2, strokeColor,
                                                               width, height, horizontalAlignment,
                                                               verticalAlignment, lineBreakMode, clipMode, maxLines,
                                                               driver);
    model->update();
    return model;
}

std::shared_ptr<VROText> VROText::createSingleLineText(std::wstring text,
                                                       std::string typefaceNames,
                                                       int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                       VROVector4f color, float extrusion,
                                                       float width, VROTextHorizontalAlignment alignment, VROTextClipMode clipMode,
                                                       std::shared_ptr<VRODriver> driver) {
    return createText(text, typefaceNames, fontSize, fontStyle, fontWeight, color, extrusion, width,
                      std::numeric_limits<float>::max(), alignment, VROTextVerticalAlignment::Center,
                      VROLineBreakMode::None, clipMode, 0, driver);
}

std::shared_ptr<VROText> VROText::createSingleLineText(std::wstring text,
                                                       std::string typefaceNames,
                                                       int fontSize, VROFontStyle fontStyle, VROFontWeight fontWeight,
                                                       VROVector4f color,
                                                       float extrusion,
                                                       std::shared_ptr<VRODriver> driver) {
    return createSingleLineText(text, typefaceNames, fontSize, fontStyle, fontWeight, color, extrusion,
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
    glyphMap[spaceCode] = firstTypeface->getGlyph(spaceCode, 0, 0, VROGlyphRenderMode::None);
    
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
                std::shared_ptr<VROGlyph> glyph = typeface->getGlyph(codePoint, 0, 0, VROGlyphRenderMode::None);
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
                 float extrusion,
                 VROTextOuterStroke stroke, int strokeWidth, VROVector4f strokeColor,
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
    _outerStroke(stroke),
    _outerStrokeWidth(strokeWidth),
    _outerStrokeColor(strokeColor),
    _driver(driver) {

}

VROText::~VROText() {
    
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
    if (_extrusion > 0.0001) {
        buildVectorizedText(_text, _typefaceCollection, _color, _extrusion, _width, _height, _horizontalAlignment, _verticalAlignment,
                            _lineBreakMode, _clipMode, _maxLines, getMaterials(), sources, elements, materials,
                            &realizedWidth, &realizedHeight, driver);
    }
    else {
        buildBitmapText(_text, _typefaceCollection, _color, _outerStroke, _outerStrokeWidth, _outerStrokeColor,
                        _width, _height, _horizontalAlignment, _verticalAlignment,
                        _lineBreakMode, _clipMode, _maxLines, sources, elements, materials,
                        &realizedWidth, &realizedHeight, driver);
    }

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

void VROText::setExtrusion(float extrusion) {
    _extrusion = extrusion;
    update();
}

void VROText::setOuterStroke(VROTextOuterStroke stroke, int outerStrokeWidth, VROVector4f outerStrokeColor) {
    _outerStroke = stroke;
    _outerStrokeWidth = outerStrokeWidth;
    _outerStrokeColor = outerStrokeColor;
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

void VROText::setMaterials(std::vector<std::shared_ptr<VROMaterial>> materials) {
    VROGeometry::setMaterials(materials);
    if (materials.size() > 0) {
        _color = materials[0]->getDiffuse().getColor();
    }
}

void VROText::buildBitmapText(std::wstring &text,
                              std::shared_ptr<VROTypefaceCollection> &typefaces,
                              VROVector4f color,
                              VROTextOuterStroke outerStroke, int outerStrokeWidth, VROVector4f outerStrokeColor,
                              float width, float height,
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
     Configure the stroke (for outline or drop shadow).
     */
    int outlineWidth;
    int outlineOffset;
    VROVector4f outlineColor = outerStrokeColor;

    if (outerStroke == VROTextOuterStroke::Outline) {
        outlineWidth = outerStrokeWidth;
        outlineOffset = -outerStrokeWidth;

        pinfo("Creating outline with color %f, %f, %f, shadow %f, %f, %f", color.x, color.y, color.z, outlineColor.x, outlineColor.y, outlineColor.z);

    } else if (outerStroke == VROTextOuterStroke::DropShadow) {
        outlineWidth = outerStrokeWidth;
        outlineOffset = 0;

        pinfo("Creating drop shadow with color %f, %f, %f, shadow %f, %f, %f", color.x, color.y, color.z, outlineColor.x, outlineColor.y, outlineColor.z);
    } else {
        outlineWidth = 0;
        outlineOffset = 0;
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
        
        std::shared_ptr<VROGlyph> whitespaceGlyph = firstTypeface->getGlyph(spaceCode, 0, outlineWidth, VROGlyphRenderMode::Bitmap);
        std::shared_ptr<VROGlyphAtlas> whitespaceAtlas = whitespaceGlyph->getBitmap(0).atlas;
        
        std::shared_ptr<VROMaterial> whitespaceMaterial = std::make_shared<VROMaterial>();
        whitespaceMaterial->setNeedsToneMapping(false);
        whitespaceMaterial->getDiffuse().setColor(color);
        whitespaceMaterial->getDiffuse().setTexture(whitespaceAtlas->getTexture());
        whitespaceMaterial->setRenderingOrder(1);
        
        std::vector<int> indices;
        materialMap[whitespaceAtlas] = { whitespaceMaterial, indices };
        
        if (outlineWidth > 0) {
            std::shared_ptr<VROGlyphAtlas> whitespaceAtlasOutline = whitespaceGlyph->getBitmap(outlineWidth).atlas;

            std::shared_ptr<VROMaterial> whitespaceMaterialOutline = std::make_shared<VROMaterial>();
            whitespaceMaterialOutline->setNeedsToneMapping(false);
            whitespaceMaterialOutline->setWritesToDepthBuffer(false);
            whitespaceMaterialOutline->getDiffuse().setColor(outlineColor);
            whitespaceMaterialOutline->getDiffuse().setTexture(whitespaceGlyph->getBitmap(outlineWidth).atlas->getTexture());
            whitespaceMaterialOutline->setRenderingOrder(0);

            std::vector<int> indices;
            materialMap[whitespaceAtlasOutline] = { whitespaceMaterialOutline, indices };
        }
        
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
                std::shared_ptr<VROGlyph> glyph = typeface->getGlyph(codePoint, 0, outlineWidth, VROGlyphRenderMode::Bitmap);
                
                const std::shared_ptr<VROGlyphAtlas> atlas = glyph->getBitmap(0).atlas;
                auto materialAndIndices = materialMap.find(atlas);
                if (materialAndIndices == materialMap.end()) {
                    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
                    material->setNeedsToneMapping(false);
                    material->getDiffuse().setColor(color);
                    material->setRenderingOrder(1);

                    std::vector<int> indices;
                    materialMap[atlas] = { material, indices };
                }
                
                if (outlineWidth > 0) {
                    const std::shared_ptr<VROGlyphAtlas> atlasOutline = glyph->getBitmap(outlineWidth).atlas;
                    auto materialAndIndices = materialMap.find(atlasOutline);
                    if (materialAndIndices == materialMap.end()) {
                        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
                        material->setNeedsToneMapping(false);
                        material->getDiffuse().setColor(outlineColor);
                        material->setRenderingOrder(0);
                        
                        // Outline strokes do not write to the depth buffer to prevent Z-fighting
                        // with the front stroke; this should be ok because the front stroke glyphs
                        // will write into the depth buffer immediately after
                        material->setWritesToDepthBuffer(false);
                        
                        std::vector<int> indices;
                        materialMap[atlasOutline] = { material, indices };
                    }
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
        const std::shared_ptr<VROGlyphAtlas> &atlas = glyph->getBitmap(0).atlas;
        std::shared_ptr<VROMaterial> material = materialMap[atlas].first;
        material->getDiffuse().setTexture(atlas->getTexture());
        
        if (outlineWidth > 0) {
            const std::shared_ptr<VROGlyphAtlas> &atlasOutline = glyph->getBitmap(outlineWidth).atlas;
            std::shared_ptr<VROMaterial> materialOutline = materialMap[atlasOutline].first;
            materialOutline->getDiffuse().setTexture(atlasOutline->getTexture());
        }
    }
    
    std::vector<VROShapeVertexLayout> var;

    VROTextFormatter::formatAndBuild(text, width, height, maxLines, maxLineHeight, horizontalAlignment, verticalAlignment, lineBreakMode, clipMode, glyphMap, outRealizedWidth, outRealizedHeight,
                                     [&var, &materialMap, outlineWidth, outlineOffset] (std::shared_ptr<VROGlyph> &glyph, float x, float y) {
                                         if (outlineWidth > 0) {
                                             const VROGlyphBitmap &bitmap = glyph->getBitmap(outlineWidth);
                                             buildBitmapChar(bitmap, x, y, outlineOffset, outlineOffset,
                                                             var, materialMap[bitmap.atlas].second);
                                         }
                                         const VROGlyphBitmap &bitmap = glyph->getBitmap(0);
                                         buildBitmapChar(bitmap, x, y, 0, 0, var, materialMap[bitmap.atlas].second);
                                     });
    buildBitmapGeometry(var, materialMap, sources, elements, materials);
}

void VROText::buildBitmapChar(const VROGlyphBitmap &bitmap,
                              float x, float y,
                              float offsetX, float offsetY,
                              std::vector<VROShapeVertexLayout> &var,
                              std::vector<int> &indices) {
    
    int index = (int)var.size();
    
    x += (bitmap.bearing.x + offsetX) * kTextPointToWorldScale;
    y += (bitmap.bearing.y - bitmap.size.y - offsetY) * kTextPointToWorldScale;
    
    float w = bitmap.size.x * kTextPointToWorldScale;
    float h = bitmap.size.y * kTextPointToWorldScale;
    
    float minU = bitmap.minU;
    float maxU = bitmap.maxU;
    float minV = bitmap.minV;
    float maxV = bitmap.maxV;
    
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

void VROText::buildBitmapGeometry(std::vector<VROShapeVertexLayout> &var,
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
    
    // Order the elements so that the front text material appears first (since this corresponds to
    // the color property)
    std::vector<std::pair<std::shared_ptr<VROMaterial>, std::vector<int>>> orderedElements;
    for (auto &kv : materialMap) {
        const std::shared_ptr<VROGlyphAtlas> &atlas = kv.first;
        if (!atlas->isOutline()) {
            orderedElements.push_back(kv.second);
        }
    }
    for (auto &kv : materialMap) {
        const std::shared_ptr<VROGlyphAtlas> &atlas = kv.first;
        if (atlas->isOutline()) {
            orderedElements.push_back(kv.second);
        }
    }
    
    // Then create the element for each
    for (auto &ordered : orderedElements) {
        std::shared_ptr<VROMaterial> &material = ordered.first;
        std::vector<int> &indices = ordered.second;
        
        std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices.data(), sizeof(int) * indices.size());
        std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                           VROGeometryPrimitiveType::Triangle,
                                                                                           indices.size() / 3,
                                                                                           sizeof(int));
        elements.push_back(element);
        materials.push_back(material);
    }
}

void VROText::buildVectorizedText(std::wstring &text,
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
                                  const std::vector<std::shared_ptr<VROMaterial>> &existingMaterials,
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
    
    /*
     Construct the materials for the front, back, and side. If materials already exist,
     do not overwrite their properties.
     */
    std::shared_ptr<VROMaterial> frontMaterial;
    if (existingMaterials.size() > 0) {
        frontMaterial = existingMaterials[0];
    } else {
        frontMaterial = std::make_shared<VROMaterial>();
        frontMaterial->getDiffuse().setColor(color);
    }
    frontMaterial->setCullMode(VROCullMode::None);
    
    std::shared_ptr<VROMaterial> backMaterial;
    if (existingMaterials.size() > 1) {
        backMaterial = existingMaterials[1];
    } else {
        backMaterial = std::make_shared<VROMaterial>();
        backMaterial->getDiffuse().setColor(color);
    }
    backMaterial->setCullMode(VROCullMode::None);
    
    std::shared_ptr<VROMaterial> sideMaterial;
    if (existingMaterials.size() > 2) {
        sideMaterial = existingMaterials[2];
    } else {
        sideMaterial = std::make_shared<VROMaterial>();
        sideMaterial->getDiffuse().setColor(color);
    }
    sideMaterial->setCullMode(VROCullMode::None);
    
    // Split the text into runs, eached mapped to a typeface
    std::vector<VROFontRun> fontRuns = typefaces->computeRuns(text);
    
    // Always add the space ' ' to the glyphMap (used by justification)
    {
        std::shared_ptr<VROTypeface> &firstTypeface = fontRuns.front().typeface;
        std::wstring space = L" ";
        uint32_t spaceCode = *space.begin();
        
        std::shared_ptr<VROGlyph> whitespaceGlyph = firstTypeface->getGlyph(spaceCode, 0, 0, VROGlyphRenderMode::Vector);
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
                std::shared_ptr<VROGlyph> glyph = typeface->getGlyph(codePoint, 0, 0, VROGlyphRenderMode::Vector);
                glyphMap[codePoint] = glyph;
            }
        }
    }
    
    std::vector<VROShapeVertexLayout> var;
    std::vector<int> frontIndices;
    std::vector<int> backIndices;
    std::vector<int> sideIndices;
    
    VROTextFormatter::formatAndBuild(text, width, height, maxLines, maxLineHeight, horizontalAlignment, verticalAlignment, lineBreakMode, clipMode, glyphMap, outRealizedWidth, outRealizedHeight,
                                     [&var, &frontIndices, &backIndices, &sideIndices, extrusion] (std::shared_ptr<VROGlyph> &glyph, float x, float y) {
                                         buildVectorizedChar(glyph, x, y, extrusion, var,
                                                             frontIndices, backIndices, sideIndices);
                                     });
    
    buildVectorizedGeometry(var, { frontIndices, backIndices, sideIndices }, sources, elements);
    materials.push_back(frontMaterial);
    materials.push_back(backMaterial);
    materials.push_back(sideMaterial);
}

void VROText::buildVectorizedChar(std::shared_ptr<VROGlyph> &glyph,
                                  float x, float y, float extrusion,
                                  std::vector<VROShapeVertexLayout> &var,
                                  std::vector<int> &frontIndices,
                                  std::vector<int> &backIndices,
                                  std::vector<int> &sideIndices) {
    
    const std::vector<VROGlyphTriangle> &triangles = glyph->getTriangles();
    for (const VROGlyphTriangle &triangle : triangles) {
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
            int index = (int) (frontIndices.size() + backIndices.size() + sideIndices.size());
            switch (triangle.getType()) {
                case VROGlyphTriangleType::Front:
                    frontIndices.push_back(index);
                    break;
                case VROGlyphTriangleType::Back:
                    backIndices.push_back(index);
                    break;
                case VROGlyphTriangleType::Side:
                    sideIndices.push_back(index);
                    break;
                default:
                    pabort();
            }
        }
    }
}

void VROText::buildVectorizedGeometry(std::vector<VROShapeVertexLayout> &var,
                                      std::vector<std::vector<int>> indices,
                                      std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                      std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    int numVertices = (int) var.size();
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>(var.data(), var.size() * sizeof(VROShapeVertexLayout));
    
    std::vector<std::shared_ptr<VROGeometrySource>> genSources = VROShapeUtilBuildGeometrySources(vertexData, numVertices);
    for (std::shared_ptr<VROGeometrySource> source : genSources) {
        sources.push_back(source);
    }
    
    for (size_t i = 0; i < indices.size(); i++) {
        std::vector<int> &elementIndices = indices[i];
        std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) elementIndices.data(), sizeof(int) * elementIndices.size());
        std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                           VROGeometryPrimitiveType::Triangle,
                                                                                           elementIndices.size() / 3,
                                                                                           sizeof(int));
        elements.push_back(element);
    }
}
