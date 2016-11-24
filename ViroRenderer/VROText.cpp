//
//  VROText.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROText.h"
#include "VROShapeUtils.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROMaterial.h"
#include "VROTypeface.h"
#include "VROGlyph.h"

// TODO Remove
#include "VROTypefaceAndroid.h"

std::shared_ptr<VROText> VROText::createText(std::string text) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;
    buildGeometry(text, sources, elements, materials);
    
    std::shared_ptr<VROText> model = std::shared_ptr<VROText>(new VROText(sources, elements));
    model->getMaterials().insert(model->getMaterials().end(), materials.begin(), materials.end());
    
    return model;
}

void VROText::buildGeometry(std::string text,
                            std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                            std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                            std::vector<std::shared_ptr<VROMaterial>> &materials) {
    
    // TODO replace with typeface
    VROTypefaceAndroid typeface("whatever");
    
    int verticesPerGlyph = 6;
    int numVertices = (int) text.size() * verticesPerGlyph;
    int varSizeBytes = sizeof(VROShapeVertexLayout) * numVertices;
    VROShapeVertexLayout var[varSizeBytes];
    
    int idx = 0;
    std::string::const_iterator c;

    float x = 0;
    for (c = text.begin(); c != text.end(); c++) {
        FT_ULong charCode = *c;
        std::unique_ptr<VROGlyph> glyph = typeface.loadGlyph(charCode);
        
        x += glyph->getBearing().x;
        float y = glyph->getSize().y - glyph->getBearing().y;
        
        float w = glyph->getSize().x;
        float h = glyph->getSize().y;
        
        var[idx + 0].x = x;
        var[idx + 0].y = y + h;
        var[idx + 0].z = 0;
        var[idx + 0].u = 0;
        var[idx + 0].v = 0;
        var[idx + 0].nx = 0;
        var[idx + 0].ny = 0;
        var[idx + 0].nz = 1;
        
        var[idx + 1].x = x;
        var[idx + 1].y = y;
        var[idx + 1].z = 0;
        var[idx + 1].u = 0;
        var[idx + 1].v = 1;
        var[idx + 1].nx = 0;
        var[idx + 1].ny = 0;
        var[idx + 1].nz = 1;
        
        var[idx + 2].x = x + w;
        var[idx + 2].y = y;
        var[idx + 2].z = 0;
        var[idx + 2].u = 1;
        var[idx + 2].v = 1;
        var[idx + 2].nx = 0;
        var[idx + 2].ny = 0;
        var[idx + 2].nz = 1;
        
        var[idx + 3].x = x;
        var[idx + 3].y = y + h;
        var[idx + 3].z = 0;
        var[idx + 3].u = 0;
        var[idx + 3].v = 0;
        var[idx + 3].nx = 0;
        var[idx + 3].ny = 0;
        var[idx + 3].nz = 1;
        
        var[idx + 4].x = x + w;
        var[idx + 4].y = y;
        var[idx + 4].z = 0;
        var[idx + 4].u = 1;
        var[idx + 4].v = 1;
        var[idx + 4].nx = 0;
        var[idx + 4].ny = 0;
        var[idx + 4].nz = 1;
        
        var[idx + 5].x = x + w;
        var[idx + 5].y = y + h;
        var[idx + 5].z = 0;
        var[idx + 5].u = 1;
        var[idx + 5].v = 0;
        var[idx + 5].nx = 0;
        var[idx + 5].ny = 0;
        var[idx + 5].nz = 1;
        
        /*
         Now advance cursors for next glyph. Each advance unit is 1/64 of a pixel,
         so divide by 64 (>> 6) to get advance in pixels.
         */
        x += (glyph->getAdvance() >> 6);
        
        int indices[verticesPerGlyph];
        for (int i = 0; i < verticesPerGlyph; i++) {
            indices[i] = idx + i;
        }
        std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * verticesPerGlyph);
        
        std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                           VROGeometryPrimitiveType::Triangle,
                                                                                           verticesPerGlyph / 3,
                                                                                           sizeof(int));
        elements.push_back(element);
        
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        material->setWritesToDepthBuffer(true);
        material->setReadsFromDepthBuffer(true);
        material->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
        material->getDiffuse().setTexture(glyph->getTexture());
        
        char name[2];
        name[0] = (char)*c;
        name[1] = 0;
        material->setName(name);
        
        materials.push_back(material);
        
        idx += 6;
    }
    
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) var, varSizeBytes);
    
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

VROText::~VROText() {
    
}
