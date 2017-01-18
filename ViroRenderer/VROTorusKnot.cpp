//
//  VROTorusKnot.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/11/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTorusKnot.h"
#include "VROShapeUtils.h"
#include "VROVector3f.h"
#include "VROData.h"
#include "VROMaterial.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"

std::shared_ptr<VROTorusKnot> VROTorusKnot::createTorusKnot(float p, float q, float tubeRadius,
                                                            int segments, int slices) {
    
    size_t vertexCount = (segments + 1) * (slices + 1);
    size_t indexCount = segments * (slices + 1) * 6;
    
    VROShapeVertexLayout *vertices = (VROShapeVertexLayout *) malloc(sizeof(VROShapeVertexLayout) * vertexCount);
    int *indices = (int *) malloc(sizeof(int) * indexCount);
    
    const float epsilon = 1e-4;
    const float dt = (2 * M_PI) / segments;
    const float du = (2 * M_PI) / slices;
    
    int vi = 0;
    for (size_t i = 0; i <= segments; ++i) {
        // calculate a point that lies on the curve
        float t0 = i * dt;
        float r0 = (2 + cosf(q * t0)) * 0.5;
        
        VROVector3f p0(r0 * cosf(p * t0),
                       r0 * sinf(p * t0),
                       -sinf(q * t0));
        
        // approximate the Frenet frame { T, N, B } for the curve at the current point
        
        float t1 = t0 + epsilon;
        float r1 = (2 + cosf(q * t1)) * 0.5;
        
        // p1 is p0 advanced infinitesimally along the curve
        VROVector3f p1(r1 * cosf(p * t1),
                       r1 * sinf(p * t1),
                       -sinf(q * t1));
        
        // compute approximate tangent as vector connecting p0 to p1
        VROVector3f T(p1.x - p0.x,
                      p1.y - p0.y,
                      p1.z - p0.z);
        
        // rough approximation of normal vector
        VROVector3f N(p1.x + p0.x,
                      p1.y + p0.y,
                      p1.z + p0.z);
        
        // compute binormal of curve
        VROVector3f B = T.cross(N);
        
        // refine normal vector by Graham-Schmidt
        N = B.cross(T);
        
        B = B.normalize();
        N = N.normalize();
        
        // generate points in a circle perpendicular to the curve at the current point
        for (size_t j = 0; j <= slices; ++j, ++vi) {
            float u = j * du;
            
            // compute position of circle point
            float x = tubeRadius * cosf(u);
            float y = tubeRadius * sinf(u);
            
            VROVector3f p2(x * N.x + y * B.x,
                           x * N.y + y * B.y,
                           x * N.z + y * B.z);
            
            vertices[vi].x = p0.x + p2.x;
            vertices[vi].y = p0.y + p2.y;
            vertices[vi].z = p0.z + p2.z;
            
            // compute normal of circle point
            VROVector3f n2 = p2.normalize();
            
            vertices[vi].nx = n2.x;
            vertices[vi].ny = n2.y;
            vertices[vi].nz = n2.z;
        }
    }
    
    // generate triplets of indices to create torus triangles
    int i = 0;
    for (int vi = 0; vi < segments * (slices + 1); ++vi) {
        indices[i++] = vi;
        indices[i++] = vi + slices + 1;
        indices[i++] = vi + slices;
        
        indices[i++] = vi;
        indices[i++] = vi + 1;
        indices[i++] = vi + slices + 1;
    }
    
    int stride = sizeof(VROShapeVertexLayout);
    
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) vertices, stride * vertexCount);
    free(vertices);
    
    std::shared_ptr<VROGeometrySource> position = std::make_shared<VROGeometrySource>(vertexData,
                                                                                      VROGeometrySourceSemantic::Vertex,
                                                                                      vertexCount,
                                                                                      true, 3,
                                                                                      sizeof(float),
                                                                                      0,
                                                                                      stride);
    std::shared_ptr<VROGeometrySource> texcoords = std::make_shared<VROGeometrySource>(vertexData,
                                                                                       VROGeometrySourceSemantic::Texcoord,
                                                                                       vertexCount,
                                                                                       true, 2,
                                                                                       sizeof(float),
                                                                                       sizeof(float) * 3,
                                                                                       stride);
    std::shared_ptr<VROGeometrySource> normal = std::make_shared<VROGeometrySource>(vertexData,
                                                                                    VROGeometrySourceSemantic::Normal,
                                                                                    vertexCount,
                                                                                    true, 3,
                                                                                    sizeof(float),
                                                                                    sizeof(float) * 5,
                                                                                    stride);
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources = { position, texcoords, normal };
    
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * indexCount);
    free(indices);

    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::Triangle,
                                                                                       indexCount / 3,
                                                                                       sizeof(int));
    std::vector<std::shared_ptr<VROGeometryElement>> elements = { element };
    
    std::shared_ptr<VROTorusKnot> torusKnot = std::shared_ptr<VROTorusKnot>(new VROTorusKnot(sources, elements));
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    torusKnot->getMaterials().push_back(material);
    
    return torusKnot;
}

VROTorusKnot::~VROTorusKnot() {
    
}


