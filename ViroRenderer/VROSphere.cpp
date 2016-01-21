//
//  VROSphere.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/20/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSphere.h"
#include "VROShapeUtils.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROData.h"
#include "VROMaterial.h"
#include "VROLog.h"
#include <vector>

std::shared_ptr<VROSphere> VROSphere::createSphere(float radius, int widthSegments, int heightSegments, bool facesOutward) {
    float phiStart = 0;
    float phiLength = M_PI * 2.0;
    
    float thetaStart = 0;
    float thetaLength = M_PI;
    float thetaEnd = thetaStart + thetaLength;
    
    int vertexCount = ((widthSegments + 1) * (heightSegments + 1));
    int varSizeBytes = sizeof(VROShapeVertexLayout) * vertexCount;
    VROShapeVertexLayout var[varSizeBytes];

    int index = 0;
    std::vector<std::vector<int>> vertices;
    
    for (int y = 0; y <= heightSegments; y++) {
        std::vector<int> verticesRow;
        float v = (float) y / (float) heightSegments;
        
        for (int x = 0; x <= widthSegments; x++) {
            float u = (float) x / (float) widthSegments;
            
            float px = -radius * cos(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);
            float py =  radius * cos(thetaStart + v * thetaLength);
            float pz =  radius * sin(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);
            
            VROVector3f normal(px, py, pz);
            normal = normal.normalize();
            
            if (!facesOutward) {
                normal.scale(-1, &normal);
            }
            
            var[index].x = px;
            var[index].y = py;
            var[index].z = pz;
            var[index].u = 1 - u;
            var[index].v = v;
            var[index].nx = normal.x;
            var[index].ny = normal.y;
            var[index].nz = normal.z;
            
            verticesRow.push_back(index);
            ++index;
        }
        
        vertices.push_back(verticesRow);
    }
    
    std::vector<int> indices;
    
    for (int y = 0; y < heightSegments; y++) {
        for (int x = 0; x < widthSegments; x++) {
            int v1 = vertices[y][x + 1];
            int v2 = vertices[y][x];
            int v3 = vertices[y + 1][x];
            int v4 = vertices[y + 1][x + 1];
            
            if (!facesOutward) {
                if (y != 0 || thetaStart > 0) {
                    indices.push_back(v1);
                    indices.push_back(v4);
                    indices.push_back(v2);
                }
                if (y != heightSegments - 1 || thetaEnd < M_PI) {
                    indices.push_back(v2);
                    indices.push_back(v4);
                    indices.push_back(v3);
                }
            }
            else {
                if (y != 0 || thetaStart > 0) {
                    indices.push_back(v1);
                    indices.push_back(v2);
                    indices.push_back(v4);
                }
                if (y != heightSegments - 1 || thetaEnd < M_PI) {
                    indices.push_back(v2);
                    indices.push_back(v3);
                    indices.push_back(v4);
                }
            }
        }
    }
    
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) var, varSizeBytes);
    std::shared_ptr<VROGeometrySource> position = std::make_shared<VROGeometrySource>(vertexData,
                                                                                      VROGeometrySourceSemantic::Vertex,
                                                                                      vertexCount,
                                                                                      true, 3,
                                                                                      sizeof(float),
                                                                                      0,
                                                                                      sizeof(VROShapeVertexLayout));
    std::shared_ptr<VROGeometrySource> texcoord = std::make_shared<VROGeometrySource>(vertexData,
                                                                                      VROGeometrySourceSemantic::Texcoord,
                                                                                      vertexCount,
                                                                                      true, 2,
                                                                                      sizeof(float),
                                                                                      sizeof(float) * 3,
                                                                                      sizeof(VROShapeVertexLayout));
    std::shared_ptr<VROGeometrySource> normal = std::make_shared<VROGeometrySource>(vertexData,
                                                                                    VROGeometrySourceSemantic::Normal,
                                                                                    vertexCount,
                                                                                    true, 3,
                                                                                    sizeof(float),
                                                                                    sizeof(float) * 5,
                                                                                    sizeof(VROShapeVertexLayout));
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources = { position, texcoord, normal };
    
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices.data(), sizeof(int) * indices.size());
    
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::Triangle,
                                                                                       indices.size() / 3,
                                                                                       sizeof(int));
    std::vector<std::shared_ptr<VROGeometryElement>> elements = { element };
    
    std::shared_ptr<VROSphere> sphere = std::shared_ptr<VROSphere>(new VROSphere(sources, elements));
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setWritesToDepthBuffer(true);
    material->setReadsFromDepthBuffer(true);
    
    sphere->getMaterials().push_back(material);
    return sphere;
}
