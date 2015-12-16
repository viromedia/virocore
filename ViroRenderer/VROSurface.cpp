//
//  VROSurface.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/3/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROSurface.h"
#include "VROData.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROMaterial.h"
#include "stdlib.h"

std::shared_ptr<VROSurface> VROSurface::createSurface(float width, float height) {
    int numVertices = 6;
    
    int varSizeBytes = sizeof(VROShapeVertexLayout) * numVertices;
    VROShapeVertexLayout var[varSizeBytes];
    
    VROSurface::buildSurface(var, width, height);
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) var, varSizeBytes);
    
    std::shared_ptr<VROGeometrySource> position = std::make_shared<VROGeometrySource>(vertexData,
                                                                                      VROGeometrySourceSemantic::Vertex,
                                                                                      numVertices,
                                                                                      3,
                                                                                      sizeof(float),
                                                                                      0,
                                                                                      sizeof(VROShapeVertexLayout));
    std::shared_ptr<VROGeometrySource> texcoord = std::make_shared<VROGeometrySource>(vertexData,
                                                                                      VROGeometrySourceSemantic::Texcoord,
                                                                                      numVertices,
                                                                                      2,
                                                                                      sizeof(float),
                                                                                      sizeof(float) * 3,
                                                                                      sizeof(VROShapeVertexLayout));
    std::shared_ptr<VROGeometrySource> normal = std::make_shared<VROGeometrySource>(vertexData,
                                                                                    VROGeometrySourceSemantic::Normal,
                                                                                    numVertices,
                                                                                    3,
                                                                                    sizeof(float),
                                                                                    sizeof(float) * 5,
                                                                                    sizeof(VROShapeVertexLayout));
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources = { position, texcoord, normal };
    
    int indices[6] = { 0, 1, 2, 3, 4, 5 };
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * 6);
    
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::Triangle,
                                                                                       2,
                                                                                       sizeof(int));
    std::vector<std::shared_ptr<VROGeometryElement>> elements = { element };
    
    std::shared_ptr<VROSurface> surface = std::shared_ptr<VROSurface>(new VROSurface(sources, elements));
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    surface->getMaterials().push_back(material);
    
    return surface;
}

void VROSurface::buildSurface(VROShapeVertexLayout *vertexLayout, float width, float height) {
    float z = 0;
    
    // Front Face
    vertexLayout[0].x = -width / 2;
    vertexLayout[0].y = -height / 2;
    vertexLayout[0].z = z;
    vertexLayout[0].u = 0;
    vertexLayout[0].v = 1;
    vertexLayout[0].nx = 0;
    vertexLayout[0].ny = 0;
    vertexLayout[0].nz = -1;
    
    vertexLayout[1].x =  width / 2;
    vertexLayout[1].y = -height / 2;
    vertexLayout[1].z = z;
    vertexLayout[1].u = 1;
    vertexLayout[1].v = 1;
    vertexLayout[1].nx = 0;
    vertexLayout[1].ny = 0;
    vertexLayout[1].nz = -1;
    
    vertexLayout[2].x = -width / 2;
    vertexLayout[2].y =  height / 2;
    vertexLayout[2].z = z;
    vertexLayout[2].u = 0;
    vertexLayout[2].v = 0;
    vertexLayout[2].nx = 0;
    vertexLayout[2].ny = 0;
    vertexLayout[2].nz = -1;
    
    vertexLayout[3].x = width / 2;
    vertexLayout[3].y = height / 2;
    vertexLayout[3].z = z;
    vertexLayout[3].u = 1;
    vertexLayout[3].v = 0;
    vertexLayout[3].nx = 0;
    vertexLayout[3].ny = 0;
    vertexLayout[3].nz = -1;
    
    vertexLayout[4].x = -width / 2;
    vertexLayout[4].y =  height / 2;
    vertexLayout[4].z = z;
    vertexLayout[4].u = 0;
    vertexLayout[4].v = 0;
    vertexLayout[4].nx = 0;
    vertexLayout[4].ny = 0;
    vertexLayout[4].nz = -1;
    
    vertexLayout[5].x =  width / 2;
    vertexLayout[5].y = -height / 2;
    vertexLayout[5].z = z;
    vertexLayout[5].u = 1;
    vertexLayout[5].v = 1;
    vertexLayout[5].nx = 0;
    vertexLayout[5].ny = 0;
    vertexLayout[5].nz = -1;
}

VROSurface::~VROSurface() {
    
}


