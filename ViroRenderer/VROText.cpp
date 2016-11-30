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
    buildGeometry(text, typeface, kTextPointToWorldScale, width, height, horizontalAlignment, verticalAlignment,
                  lineBreakMode, maxLines, sources, elements, materials, &realizedWidth, &realizedHeight);
    
    std::shared_ptr<VROText> model = std::shared_ptr<VROText>(new VROText(sources, elements, width, height));
    model->getMaterials().insert(model->getMaterials().end(), materials.begin(), materials.end());
    
    return model;
}

void VROText::buildGeometry(std::string text,
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
    
    std::vector<VROShapeVertexLayout> var;
    
    float x = 0;
    
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        FT_ULong charCode = *c;
        std::unique_ptr<VROGlyph> glyph = typeface->loadGlyph(charCode);
        
        x += glyph->getBearing().x * scale;
        float y = (glyph->getBearing().y - glyph->getSize().y) * scale;
        
        float w = glyph->getSize().x * scale;
        float h = glyph->getSize().y * scale;
        
        *outRealizedHeight = std::max(*outRealizedHeight, h);
        
        buildChar(var, (char)*c, x, y, w, h,
                  glyph->getMinU(), glyph->getMaxU(), glyph->getMinV(), glyph->getMaxV(),
                  glyph->getTexture(), elements, materials);
        
        
        /*
         Now advance cursors for next glyph. Each advance unit is 1/64 of a pixel,
         so divide by 64 (>> 6) to get advance in pixels.
         */
        x += (glyph->getAdvance() >> 6) * scale;
    }
    
    *outRealizedWidth = x;
    
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
}

void VROText::buildChar(std::vector<VROShapeVertexLayout> &var,
                        char c, float x, float y, float w, float h,
                        float minU, float maxU, float minV, float maxV,
                        std::shared_ptr<VROTexture> texture,
                        std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                        std::vector<std::shared_ptr<VROMaterial>> &materials) {
    
    int index = (int)var.size();
    
    var.push_back({x,     y + h, 0, minU, minV, 0, 0, 1});
    var.push_back({x,     y,     0, minU, maxV, 0, 0, 1});
    var.push_back({x + w, y,     0, maxU, maxV, 0, 0, 1});
    var.push_back({x,     y + h, 0, minU, minV, 0, 0, 1});
    var.push_back({x + w, y,     0, maxU, maxV, 0, 0, 1});
    var.push_back({x + w, y + h, 0, maxU, minV, 0, 0, 1});
    
    /*
     Make the index array and element.
     */
    int indices[kVerticesPerGlyph];
    for (int i = 0; i < kVerticesPerGlyph; i++) {
        indices[i] = index + i;
    }
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * kVerticesPerGlyph);
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::Triangle,
                                                                                       kVerticesPerGlyph / 3,
                                                                                       sizeof(int));
    elements.push_back(element);
    
    /*
     Make the corresponding material.
     */
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setWritesToDepthBuffer(true);
    material->setReadsFromDepthBuffer(true);
    material->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    material->getDiffuse().setTexture(texture);
    
    char name[2] = { c, 0 };
    material->setName(name);
    
    materials.push_back(material);
}

VROText::~VROText() {
    
}
