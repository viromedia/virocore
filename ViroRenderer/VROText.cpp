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

static const int kVerticesPerGlyph = 6;
static const float kTextPointToWorldScale = 0.05;

std::shared_ptr<VROText> VROText::createText(std::string text, std::shared_ptr<VROTypeface> typeface, float width, float height,
                                             VROTextHorizontalAlignment horizontalAlignment, VROTextVerticalAlignment verticalAlignment,
                                             VROLineBreakMode lineBreakMode, int maxLines) {
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;
    
    float realizedWidth, realizedHeight;
    buildText(text, typeface, kTextPointToWorldScale, width, height, horizontalAlignment, verticalAlignment,
              lineBreakMode, maxLines, sources, elements, materials, &realizedWidth, &realizedHeight);
    
    std::shared_ptr<VROText> model = std::shared_ptr<VROText>(new VROText(sources, elements, width, height));
    model->getMaterials().insert(model->getMaterials().end(), materials.begin(), materials.end());
    
    return model;
}

void VROText::buildText(std::string text,
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
     Build the geometry of the text into the var vector, while updating the
     associated indices in the materialMap.
     */
    std::vector<VROShapeVertexLayout> var;
    std::map<FT_ULong, std::shared_ptr<VROGeometryElement>> elementMap;
    
    float x = 0;
    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
        FT_ULong charCode = *c;
        std::unique_ptr<VROGlyph> &glyph = glyphMap[charCode];
        
        x += glyph->getBearing().x * scale;
        //*outRealizedHeight = std::max(*outRealizedHeight, h);
        
        buildChar(glyph, x, 0, scale, var, materialMap[charCode].second);
        
        /*
         Now advance cursors for next glyph. Each advance unit is 1/64 of a pixel,
         so divide by 64 (>> 6) to get advance in pixels.
         */
        x += (glyph->getAdvance() >> 6) * scale;
    }
    
    //*outRealizedWidth = x;
    buildGeometry(var, materialMap, sources, elements, materials);
}

void VROText::buildChar(std::unique_ptr<VROGlyph> &glyph,
                        float x, float y, float scale,
                        std::vector<VROShapeVertexLayout> &var,
                        std::vector<int> &indices) {
    
    int index = (int)var.size();
    
    x += glyph->getBearing().x * scale;
    y += (glyph->getBearing().y - glyph->getSize().y) * scale;
    
    float w = glyph->getSize().x * scale;
    float h = glyph->getSize().y * scale;
    
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

VROText::~VROText() {
    
}
