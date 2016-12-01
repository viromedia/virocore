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
#include "VROGlyph.h"
#include "VROLog.h"
#include <cstddef>
#include "VROStringUtil.h"

static const int kVerticesPerGlyph = 6;
static const float kTextPointToWorldScale = 0.05;
static const std::string kWhitespaceDelimeters = " \n\t\v\r";

std::shared_ptr<VROText> VROText::createText(std::string text, std::shared_ptr<VROTypeface> typeface, float width, float height,
                                             VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                             VROLineBreakMode lineBreakMode, int maxLines) {
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;
    
    float realizedWidth, realizedHeight;
    buildText(text, typeface, width, height, horizontalAlignment, verticalAlignment,
              lineBreakMode, maxLines, sources, elements, materials, &realizedWidth, &realizedHeight);
    
    std::shared_ptr<VROText> model = std::shared_ptr<VROText>(new VROText(sources, elements, width, height));
    model->getMaterials().insert(model->getMaterials().end(), materials.begin(), materials.end());
    
    return model;
}

void VROText::buildText(std::string &text,
                        std::shared_ptr<VROTypeface> &typeface,
                        float width,
                        float height,
                        VROTextHorizontalAlignment horizontalAlignment,
                        VROTextVerticalAlignment verticalAlignment,
                        VROLineBreakMode lineBreakMode,
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
    std::map<FT_ULong, std::unique_ptr<VROGlyph>> glyphMap;
    std::map<FT_ULong, std::pair<std::shared_ptr<VROMaterial>, std::vector<int>>> materialMap;
    
    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
        FT_ULong charCode = *c;
        if (glyphMap.find(charCode) == glyphMap.end()) {
            std::unique_ptr<VROGlyph> glyph = typeface->loadGlyph(charCode);
            
            std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
            material->setWritesToDepthBuffer(true);
            material->setReadsFromDepthBuffer(true);
            material->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
            material->getDiffuse().setTexture(glyph->getTexture());
            
            char name[2] = { (char)charCode, 0 };
            material->setName(name);
            
            std::vector<int> indices;
            materialMap[charCode] = {material, indices};
            glyphMap[charCode] = std::move(glyph);
        }
    }
    
    /*
     Divide the text into its individual lines.
     */
    std::vector<std::string> lines = (lineBreakMode == VROLineBreakMode::WordWrap) ?
        wrapByWords(text, width, maxLines, typeface, glyphMap) :
        wrapByChars(text, width, maxLines, glyphMap);
    
    float lineHeight = typeface->getLineHeight() * kTextPointToWorldScale;
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
    for (std::string &line : lines) {
        /*
         Compute the width of the line.
         */
        float lineWidth = 0;
        for (std::string::const_iterator c = line.begin(); c != line.end(); ++c) {
            FT_ULong charCode = *c;
            std::unique_ptr<VROGlyph> &glyph = glyphMap[charCode];
            
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
        
        for (std::string::const_iterator c = line.begin(); c != line.end(); ++c) {
            FT_ULong charCode = *c;
            std::unique_ptr<VROGlyph> &glyph = glyphMap[charCode];
            
            buildChar(glyph, x, y, var, materialMap[charCode].second);
            x += glyph->getAdvance() * kTextPointToWorldScale;
        }
        
        y -= lineHeight;
        *outRealizedWidth = std::max(*outRealizedWidth, lineWidth);
    }
    
    buildGeometry(var, materialMap, sources, elements, materials);
    *outRealizedHeight = totalHeight;
}

void VROText::buildChar(std::unique_ptr<VROGlyph> &glyph,
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
                            std::map<FT_ULong, std::pair<std::shared_ptr<VROMaterial>, std::vector<int>>> &materialMap,
                            std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                            std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                            std::vector<std::shared_ptr<VROMaterial>> &materials) {
    
    int numVertices = (int) var.size();
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>(var.data(), var.size() * sizeof(VROShapeVertexLayout));
    
    std::shared_ptr<VROGeometrySource> position = std::make_shared<VROGeometrySource>(vertexData,
                                                                                      VROGeometrySourceSemantic::Vertex,
                                                                                      numVertices,
                                                                                      true, 3,
                                                                                      sizeof(float),
                                                                                      0,
                                                                                      sizeof(VROShapeVertexLayout));
    std::shared_ptr<VROGeometrySource> texcoord = std::make_shared<VROGeometrySource>(vertexData,
                                                                                      VROGeometrySourceSemantic::Texcoord,
                                                                                      numVertices,
                                                                                      true, 2,
                                                                                      sizeof(float),
                                                                                      sizeof(float) * 3,
                                                                                      sizeof(VROShapeVertexLayout));
    std::shared_ptr<VROGeometrySource> normal = std::make_shared<VROGeometrySource>(vertexData,
                                                                                    VROGeometrySourceSemantic::Normal,
                                                                                    numVertices,
                                                                                    true, 3,
                                                                                    sizeof(float),
                                                                                    sizeof(float) * 5,
                                                                                    sizeof(VROShapeVertexLayout));
    sources.push_back(position);
    sources.push_back(texcoord);
    sources.push_back(normal);
    
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

std::vector<std::string> VROText::wrapByWords(std::string &text, int maxWidth, int maxLines,
                                              std::shared_ptr<VROTypeface> &typeface,
                                              std::map<FT_ULong, std::unique_ptr<VROGlyph>> &glyphMap) {
    
    
    std::vector<std::string> lines;
    float lineWidth = 0;
    std::string currentLine;
    
    size_t current = 0;
    while (true) {
        size_t delimeterStart = text.find_first_of(kWhitespaceDelimeters, current);
        size_t delimeterEnd;
        
        if (delimeterStart == std::string::npos) {
            delimeterEnd = text.size();
        }
        else {
            delimeterEnd = text.find_first_not_of(kWhitespaceDelimeters, delimeterStart);
        }
        
        std::string word = text.substr(current, delimeterEnd - current);
        
        if (!word.empty()) {
            float wordWidth = 0;
            for (std::string::const_iterator c = word.begin(); c != word.end(); ++c) {
                FT_ULong charCode = *c;
                std::unique_ptr<VROGlyph> &glyph = glyphMap[charCode];
                
                wordWidth += glyph->getAdvance() * kTextPointToWorldScale;
            }
            
            if (lineWidth + wordWidth > maxWidth) {
                /*
                 If true, then the word is too large for an empty line,
                 so place it in its own line and move on.
                 */
                if (currentLine.empty()) {
                    lines.push_back(word);
                    current = delimeterEnd;
                }
                
                /*
                 Otherwise finish the current line and try placing the word
                 again, with a fresh line.
                 */
                else {
                    lines.push_back(currentLine);
                    lineWidth = 0;
                    currentLine.clear();
                }
                
                if (maxLines > 0 && lines.size() >= maxLines) {
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
        
        if (current == text.size()) {
            break;
        }
    }
    
    if (!currentLine.empty() && (maxLines == 0 || lines.size() < maxLines)) {
        lines.push_back(currentLine);
    }

    return lines;
}

std::vector<std::string> VROText::wrapByChars(std::string &text, int maxWidth, int maxLines,
                                              std::map<FT_ULong, std::unique_ptr<VROGlyph>> &glyphMap) {
    
    
    std::vector<std::string> lines;
    float lineWidth = 0;
    std::string currentLine;
    
    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
        FT_ULong charCode = *c;
        std::unique_ptr<VROGlyph> &glyph = glyphMap[charCode];
        
        float charWidth = glyph->getAdvance() * kTextPointToWorldScale;
        if (lineWidth + charWidth > maxWidth) {
            lines.push_back(currentLine);
            if (maxLines > 0 && lines.size() >= maxLines) {
                break;
            }
            
            currentLine.clear();
            lineWidth = 0;
            --c;
        }
        else {
            lineWidth += charWidth;
            currentLine += *c;
        }
    }
    
    if (!currentLine.empty() && (maxLines == 0 || lines.size() < maxLines)) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

VROText::~VROText() {
    
}
