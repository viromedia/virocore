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
#include "VROStringUtil.h"
#include "VROKnuthPlassFormatter.h"
#include <ft2build.h>
#include FT_FREETYPE_H

static const int kVerticesPerGlyph = 6;
static const float kTextPointToWorldScale = 0.01;
static const std::wstring kWhitespaceDelimeters = L" \t\v\r";

static const int kJustificationToleranceStart = 2;
static const int kJustificationToleranceEnd = 4;

std::shared_ptr<VROText> VROText::createText(std::wstring text,
                                             std::shared_ptr<VROTypefaceCollection> typefaces,
                                             VROVector4f color, float width, float height,
                                             VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                             VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines) {
    
    std::shared_ptr<VROText> model = std::make_shared<VROText>(text, typefaces, color, width, height, horizontalAlignment,
                                                               verticalAlignment, lineBreakMode, clipMode, maxLines);
    model->update();
    return model;
}

std::shared_ptr<VROText> VROText::createSingleLineText(std::wstring text,
                                                       std::shared_ptr<VROTypefaceCollection> typefaces,
                                                       VROVector4f color,
                                                       float width, VROTextHorizontalAlignment alignment, VROTextClipMode clipMode) {
    return createText(text, typefaces, color, width, std::numeric_limits<float>::max(), alignment, VROTextVerticalAlignment::Center,
                      VROLineBreakMode::None, clipMode);
}

std::shared_ptr<VROText> VROText::createSingleLineText(std::wstring text, std::shared_ptr<VROTypefaceCollection> typefaces,
                                                       VROVector4f color) {
    return createSingleLineText(text, typefaces, color, std::numeric_limits<float>::max(), VROTextHorizontalAlignment::Center,
                                VROTextClipMode::None);
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
            lines = wrapByWords(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::CharWrap:
            lines = wrapByChars(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::None:
            lines = wrapByNewlines(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::Justify:
            lines = justify(text, maxWidth, maxHeight, maxLines, maxLineHeight, clipMode, glyphMap);
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
                 std::shared_ptr<VROTypefaceCollection> typefaces,
                 VROVector4f color,
                 float width, float height,
                 VROTextHorizontalAlignment horizontalAlignment,
                 VROTextVerticalAlignment verticalAlignment,
                 VROLineBreakMode lineBreakMode, VROTextClipMode clipMode, int maxLines) :
    _text(text),
    _typefaces(typefaces),
    _color(color),
    _width(width),
    _height(height),
    _horizontalAlignment(horizontalAlignment),
    _verticalAlignment(verticalAlignment),
    _lineBreakMode(lineBreakMode),
    _clipMode(clipMode),
    _maxLines(maxLines) {

}

void VROText::update() {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;

    float realizedWidth, realizedHeight;
    buildText(_text, _typefaces, _color, _width, _height, _horizontalAlignment, _verticalAlignment,
              _lineBreakMode, _clipMode, _maxLines, sources, elements, materials,
              &realizedWidth, &realizedHeight);

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

void VROText::setTypefaceCollection(std::shared_ptr<VROTypefaceCollection> typefaces) {
    _typefaces = typefaces;
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
                        float *outRealizedWidth, float *outRealizedHeight) {
    
    /*
     Create a glyph, material, and vector of indices for each character
     in the text string. If a character appears multiple times in the text,
     we can share the same glyph, material, and indices vector across them
     all.
     */
    std::map<uint32_t, std::shared_ptr<VROGlyph>> glyphMap;
    std::map<uint32_t, std::pair<std::shared_ptr<VROMaterial>, std::vector<int>>> materialMap;
    
    // Split the text into runs, eached mapped to a typeface
    std::vector<VROFontRun> fontRuns = typefaces->computeRuns(text);
    
    // Always add the space ' ' to the glyphMap (used by justification)
    {
        std::shared_ptr<VROTypeface> &firstTypeface = fontRuns.front().typeface;
        std::wstring space = L" ";
        uint32_t spaceCode = *space.begin();
        
        std::shared_ptr<VROGlyph> whitespaceGlyph = firstTypeface->getGlyph(spaceCode, 0, true);
        
        std::shared_ptr<VROMaterial> whitespaceMaterial = std::make_shared<VROMaterial>();
        whitespaceMaterial->getDiffuse().setColor(color);
        whitespaceMaterial->getDiffuse().setTexture(whitespaceGlyph->getTexture());
        
        char whitespaceName[2] = { (char)spaceCode, 0 };
        whitespaceMaterial->setName(whitespaceName);
        
        std::vector<int> indices;
        materialMap[spaceCode] = { whitespaceMaterial, indices };
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
                
                std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
                material->getDiffuse().setColor(color);
                material->getDiffuse().setTexture(glyph->getTexture());
                
                char name[2] = { (char)codePoint, 0 };
                material->setName(name);
                
                std::vector<int> indices;
                materialMap[codePoint] = { material, indices };
                glyphMap[codePoint] = glyph;
            }
        }
    }
    
    /*
     Divide the text into its individual lines.
     */
    std::vector<VROTextLine> lines;
    switch (lineBreakMode) {
        case VROLineBreakMode::WordWrap:
            lines = wrapByWords(text, width, height, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::CharWrap:
            lines = wrapByChars(text, width, height, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::None:
            lines = wrapByNewlines(text, width, height, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        case VROLineBreakMode::Justify:
            lines = justify(text, width, height, maxLines, maxLineHeight, clipMode, glyphMap);
            break;
        default:
            pabort("Invalid linebreak mode found for VROText");
            break;
    }
    
    float lineHeight = maxLineHeight * kTextPointToWorldScale;
    float totalHeight = lines.size() * lineHeight;

    /*
     Compute the Y starting point for the text based on the vertical 
     alignment setting.
     */
    float y = 0;
    if (verticalAlignment == VROTextVerticalAlignment::Top) {
        y = height / 2.0 - lineHeight;
    }
    else if (verticalAlignment == VROTextVerticalAlignment::Bottom) {
        y = -height / 2.0 + totalHeight - lineHeight;
    }
    else { // Center
        y = totalHeight / 2.0 - lineHeight / 2.0;
    }
    
    /*
     Build the geometry of the text into the var vector, while updating the
     associated indices in the materialMap.
     */
    std::vector<VROShapeVertexLayout> var;
    for (VROTextLine &textLine : lines) {
        std::wstring &line = textLine.line;
        
        /*
         Compute the width of the line.
         */
        float lineWidth = 0;
        for (std::wstring::const_iterator c = line.begin(); c != line.end(); ++c) {
            uint32_t charCode = *c;
            std::shared_ptr<VROGlyph> &glyph = glyphMap[charCode];
            
            lineWidth += glyph->getAdvance() * kTextPointToWorldScale;
        }
        
        /*
         Compute the X starting point for the text based on the
         horizontal alignment setting.
         */
        float x = 0;
        if (horizontalAlignment == VROTextHorizontalAlignment::Left) {
            x = -width / 2.0;
        }
        else if (horizontalAlignment == VROTextHorizontalAlignment::Right) {
            x = width / 2.0 - lineWidth;
        }
        else { // Center
            x = -lineWidth / 2.0;
        }
        
        for (std::wstring::const_iterator c = line.begin(); c != line.end(); ++c) {
            uint32_t charCode = *c;
            std::shared_ptr<VROGlyph> &glyph = glyphMap[charCode];
            
            buildChar(glyph, x, y, var, materialMap[charCode].second);
            float advance = glyph->getAdvance() * kTextPointToWorldScale;
            
            if (charCode == ' ') {
                advance *= textLine.spacingRatio;
            }
            
            x += advance;
        }
        
        y -= lineHeight;
        *outRealizedWidth = std::max(*outRealizedWidth, lineWidth);
    }
    
    buildGeometry(var, materialMap, sources, elements, materials);
    *outRealizedHeight = totalHeight;
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
                            std::map<uint32_t, std::pair<std::shared_ptr<VROMaterial>, std::vector<int>>> &materialMap,
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

std::vector<VROTextLine> VROText::wrapByWords(std::wstring &text,
                                              float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                              VROTextClipMode clipMode,
                                              std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap) {
    
    /*
     NOTE: the greedy algorithm used here combines hard breaks with soft breaks, which
     makes it more unwieldy but more efficient. If we need to add functionality and it
     gets more out of hand, it may be worthwhile to simplify this function by instead
     first dividing the string up into paragraphs based on hard breaks ('\n' characters), 
     and then wrapping each paragraph individually.
     */
    
    std::vector<VROTextLine> lines;
    float lineWidth = 0;
    std::wstring currentLine;
    
    size_t current = 0;
    while (true) {
        if (current == text.size() || current == std::wstring::npos) {
            break;
        }
        
        /*
         If the first character is a hard break, process it.
         */
        if (text[current] == '\n') {
            lines.push_back({currentLine});
            lineWidth = 0;
            currentLine.clear();
            
            if (!isAnotherLineAvailable(lines.size(), maxHeight, maxLines, lineHeight, clipMode)) {
                break;
            }
            
            ++current;
            continue;
        }
        
        size_t delimeterStart = text.find_first_of(kWhitespaceDelimeters, current);
        size_t delimeterEnd;
        size_t newlinePos = text.find_first_of('\n', current);
        
        if (delimeterStart == std::wstring::npos) {
            delimeterEnd = text.size();
        }
        else {
            delimeterEnd = text.find_first_not_of(kWhitespaceDelimeters, delimeterStart);
        }
        
        /*
         If we found a hard break ('\n'), set the end of the word to the newline
         character, so the newline character is the first thing we pick up next time
         around the loop.
         */
        if (newlinePos <= delimeterEnd) {
            delimeterEnd = newlinePos;
        }
        
        std::wstring word = text.substr(current, delimeterEnd - current);
        if (!word.empty()) {
            float wordWidth = getLengthOfWord(word, glyphMap);
            if (lineWidth + wordWidth > maxWidth) {
                /*
                 If true, then the word is too large for an empty line,
                 so place it in its own line and move on.
                 
                 Note: this causes an issue if we're clipping to bounds,
                 in that this word will breach maxWidth. Solution is not
                 obvious, though, as clipping the word mid-way would look
                 worse, as would omitting the word entirely.
                 */
                if (currentLine.empty()) {
                    lines.push_back({word});
                    current = delimeterEnd;
                }
                
                /*
                 Otherwise finish the current line and try placing the word
                 again, with a fresh line.
                 */
                else {
                    lines.push_back({currentLine});
                    lineWidth = 0;
                    currentLine.clear();
                }
                
                if (!isAnotherLineAvailable(lines.size(), maxHeight, maxLines, lineHeight, clipMode)) {
                    break;
                }
            }
            else {
                lineWidth += wordWidth;
                currentLine += word;
                
                current = delimeterEnd;
            }
        }
        else {
            current = delimeterEnd;
        }
    }
    
    if (!currentLine.empty() && isAnotherLineAvailable(lines.size(), maxHeight, maxLines, lineHeight, clipMode)) {
        lines.push_back({currentLine});
    }

    return lines;
}

std::vector<VROTextLine> VROText::wrapByChars(std::wstring &text,
                                              float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                              VROTextClipMode clipMode,
                                              std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap) {
    
    std::vector<VROTextLine> lines;
    float lineWidth = 0;
    std::wstring currentLine;
    
    for (std::wstring::const_iterator c = text.begin(); c != text.end(); ++c) {
        uint32_t charCode = *c;
        std::shared_ptr<VROGlyph> &glyph = glyphMap[charCode];
        
        float charWidth = glyph->getAdvance() * kTextPointToWorldScale;
        if (lineWidth + charWidth > maxWidth || charCode == '\n') {
            lines.push_back({currentLine});
            if (!isAnotherLineAvailable(lines.size(), maxHeight, maxLines, lineHeight, clipMode)) {
                break;
            }
            
            currentLine.clear();
            lineWidth = 0;
            
            if (charCode != '\n') {
                --c;
            }
        }
        else {
            lineWidth += charWidth;
            currentLine += *c;
        }
    }
    
    if (!currentLine.empty() && isAnotherLineAvailable(lines.size(), maxHeight, maxLines, lineHeight, clipMode)) {
        lines.push_back({currentLine});
    }
    
    return lines;
}

std::vector<VROTextLine> VROText::wrapByNewlines(std::wstring &text,
                                                 float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                                 VROTextClipMode clipMode,
                                                 std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap) {
 
    std::vector<VROTextLine> lines;
    float lineWidth = 0;
    std::wstring currentLine;
    
    for (std::wstring::const_iterator c = text.begin(); c != text.end(); ++c) {
        uint32_t charCode = *c;
        std::shared_ptr<VROGlyph> &glyph = glyphMap[charCode];
        
        float charWidth = glyph->getAdvance() * kTextPointToWorldScale;
        /*
         In this wrapping mode, we only create new lines when we encounter a
         '\n' character.
         */
        if (charCode == '\n') {
            lines.push_back({currentLine});
            if (!isAnotherLineAvailable(lines.size(), maxHeight, maxLines, lineHeight, clipMode)) {
                break;
            }
            
            currentLine.clear();
            lineWidth = 0;
        }
        
        /*
         We clip horizontally in this wrapping mode. In other wrapping modes
         this isn't necessary since we automatically wrap to a new line when
         the maxWidth is breached; therefore in other wrapping modes we only
         need to clip vertically.
         */
        else if (clipMode == VROTextClipMode::None || lineWidth + charWidth < maxWidth) {
            lineWidth += charWidth;
            currentLine += *c;
        }
    }
    
    if (!currentLine.empty() && isAnotherLineAvailable(lines.size(), maxHeight, maxLines, lineHeight, clipMode)) {
        lines.push_back({currentLine});
    }
    
    return lines;
}

std::vector<VROTextLine> VROText::justify(std::wstring &text,
                                          float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                          VROTextClipMode clipMode,
                                          std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap) {
    std::vector<VROTextLine> lines;
    
    std::wstring space = L" ";
    float spaceWidth = getLengthOfWord(space, glyphMap);
    float spaceStretch = (spaceWidth * 3) / 6;
    float spaceShrink  = (spaceWidth * 3) / 9;
    
    std::vector<std::wstring> paragraphs = divideIntoParagraphs(text);
    for (std::wstring &paragraph : paragraphs) {
        /*
         Handle empty newlines.
         */
        if (paragraph.empty()) {
            lines.push_back({paragraph, 1.0});
            continue;
        }
        
        std::vector<float> lineLengths = { maxWidth };
        
        std::vector<std::shared_ptr<KPNode>> nodes;
        std::vector<std::wstring> words = VROStringUtil::split(paragraph, kWhitespaceDelimeters, false);
        
        int index = 0;
        for (std::wstring &word : words) {
            float length = getLengthOfWord(word, glyphMap);
            nodes.push_back(std::make_shared<KPBox>(length, word));
            
            if (index == words.size() - 1) {
                nodes.push_back(std::make_shared<KPGlue>(0, kInfinity, 0, L""));
                nodes.push_back(std::make_shared<KPPenalty>(0, -kInfinity, 1));
            }
            else {
                nodes.push_back(std::make_shared<KPGlue>(spaceWidth, spaceStretch, spaceShrink, L" "));
            }
            
            ++index;
        }

        std::vector<VROBreakpoint> breaks;
        for (int tolerance = kJustificationToleranceStart; tolerance <= kJustificationToleranceEnd; tolerance++) {
            VROKnuthPlassFormatter formatter(nodes, lineLengths, tolerance);
            breaks = formatter.run();

            if (!breaks.empty()) {
                break;
            } // Otherwise loop, increasing tolerance
        }

        /*
         If we still fail to justify after trying our max tolerance, revert to standard word
         wrapping for this paragraph.
         */
        if (breaks.empty()) {
            pinfo("Failed to justify paragraph using tolerances %d through %d, falling back to greedy word wrapping",
                  kJustificationToleranceStart, kJustificationToleranceEnd);
            std::vector<VROTextLine> paragraphLines = wrapByWords(paragraph, maxWidth, maxHeight, maxLines,
                                                                  lineHeight, clipMode, glyphMap);
            lines.insert(lines.end(), paragraphLines.begin(), paragraphLines.end());
        }
        
        /*
         Construct the lines.
         */
        std::vector<std::vector<std::shared_ptr<KPNode>>> formattedLines;
        int lineStart = 0;

        for (int i = 1; i < breaks.size(); i++) {
            int point = breaks[i].position;
            
            for (int j = lineStart; j < nodes.size(); j ++) {
                // After a line break, we skip any nodes unless they are boxes or forced breaks.
                if (nodes[j]->type == KPNodeType::Box ||
                   (nodes[j]->type == KPNodeType::Penalty && std::dynamic_pointer_cast<KPPenalty>(nodes[j])->penalty == -kInfinity)) {
                    
                    lineStart = j;
                    break;
                }
            }
            
            std::vector<std::shared_ptr<KPNode>>::iterator start = nodes.begin() + lineStart;
            std::vector<std::shared_ptr<KPNode>>::iterator end   = nodes.begin() + (point + 1);
            
            formattedLines.push_back({start, end});
            lineStart = point;
        }
        
        for (int i = 0; i < formattedLines.size(); i++) {
            std::vector<std::shared_ptr<KPNode>> &nodesOnLine = formattedLines[i];
            
            std::wstring line = L"";
            int numSpaces = 0;
            float textWidth = 0;
            
            for (std::shared_ptr<KPNode> &node : nodesOnLine) {
                if (node->value == L" ") {
                    numSpaces++;
                }
                
                line += node->value;
                textWidth += getLengthOfWord(node->value, glyphMap);
            }
            
            // Don't justify the last line of a paragraph
            if (i == formattedLines.size() - 1) {
                lines.push_back({line, 1.0});
            }
            
            // Otherwise, to justify, compute how large each whitespace should be
            else {
                float spacePerWhitespace = (maxWidth - textWidth) / (float) numSpaces;
                float ratio = 1.0 + spacePerWhitespace / spaceWidth;
                
                lines.push_back({line, ratio});
            }
        }
    }

    /*
     Finally, handle clipping.
     */
    int maxLinesByClipping = std::numeric_limits<int>::max();
    if (clipMode == VROTextClipMode::ClipToBounds) {
        float lineHeightWorld = lineHeight * kTextPointToWorldScale;
        maxLinesByClipping = floorf(maxHeight / lineHeightWorld);
    }

    int maxLinesByLimit = std::numeric_limits<int>::max();
    if (maxLines != 0) {
        maxLinesByLimit = maxLines;
    }

    int linesToKeep = std::min(maxLinesByClipping, maxLinesByLimit);
    if (linesToKeep < lines.size()) {
        return { lines.begin(), lines.begin() + linesToKeep };
    }
    else {
        return lines;
    }
}

std::vector<std::wstring> VROText::divideIntoParagraphs(std::wstring &text) {
    std::vector<std::wstring> lines;
    std::wstring delimeters = L"\n";
    
    return VROStringUtil::split(text, delimeters, true);
}

float VROText::getLengthOfWord(const std::wstring &word, std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap) {
    float wordWidth = 0;
    for (std::wstring::const_iterator c = word.begin(); c != word.end(); ++c) {
        uint32_t charCode = *c;
        std::shared_ptr<VROGlyph> &glyph = glyphMap[charCode];
        
        wordWidth += glyph->getAdvance() * kTextPointToWorldScale;
    }
    
    return wordWidth;
}

bool VROText::isAnotherLineAvailable(size_t numLinesNow, float maxHeight, int maxLines,
                                     float lineHeight, VROTextClipMode clipMode) {
    // Checks both the maxLines condition and the clipping condition
    return (maxLines <= 0 || numLinesNow < maxLines) &&
           (clipMode == VROTextClipMode::None || (numLinesNow + 1) * lineHeight * kTextPointToWorldScale < maxHeight);
}


VROText::~VROText() {
    
}
