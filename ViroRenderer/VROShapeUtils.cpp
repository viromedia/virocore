//
//  VROShapeUtils.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/3/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROShapeUtils.h"
#include "VROVector3f.h"
#include "VROGeometrySource.h"
#include "VROLog.h"

void VROShapeUtilComputeTangents(VROShapeVertexLayout *vertexLayout, size_t verticesLength,
                                 int *indices, size_t indicesLength) {
    
    VROVector3f *tan1 = VROShapeUtilStartTangents(vertexLayout, verticesLength);
    VROShapeUtilComputeTangentsForIndices(vertexLayout, verticesLength, indices, indicesLength, tan1);
    VROShapeUtilEndTangents(vertexLayout, verticesLength, tan1);
}

VROVector3f *VROShapeUtilStartTangents(VROShapeVertexLayout *vertexLayout, size_t verticesLength) {
    // Zero out the tangent for all vertices
    for (size_t i = 0; i < verticesLength; i++) {
        VROShapeVertexLayout &v = vertexLayout[i];
        v.tx = 0;
        v.ty = 0;
        v.tz = 0;
        v.tw = 0;
    }
    
    VROVector3f *tan1 = new VROVector3f[verticesLength * 2];
    return tan1;
}

void VROShapeUtilComputeTangentsForIndices(VROShapeVertexLayout *vertexLayout, size_t verticesLength,
                                           int *indices, size_t indicesLength, VROVector3f *tan1) {
    
    VROVector3f *tan2 = tan1 + verticesLength;
    
    // For each triangle, compute the tangent and bitangent
    for (size_t i = 0; i < indicesLength; i += 3) {
        int i1 = indices[i + 0];
        int i2 = indices[i + 1];
        int i3 = indices[i + 2];
        
        VROShapeVertexLayout &var1 = vertexLayout[i1];
        VROShapeVertexLayout &var2 = vertexLayout[i2];
        VROShapeVertexLayout &var3 = vertexLayout[i3];
        
        VROVector3f v1(var1.x, var1.y, var1.z);
        VROVector3f v2(var2.x, var2.y, var2.z);
        VROVector3f v3(var3.x, var3.y, var3.z);

        VROVector3f w1(var1.u, var1.v, 0);
        VROVector3f w2(var2.u, var2.v, 0);
        VROVector3f w3(var3.u, var3.v, 0);
        
        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;
        
        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;
        
        float r = 1.0f / (s1 * t2 - s2 * t1);
        VROVector3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        VROVector3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
        
        // ADD the tangent to the existing value; this way we average across all triangles
        // that share this vertex. Note we do not normalize, and that is intentional: we
        // want larger triangles to have a bigger "impact" than smaller triangles
        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;
        
        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }
}

void VROShapeUtilEndTangents(VROShapeVertexLayout *vertexLayout, size_t verticesLength, VROVector3f *tan1) {
    VROVector3f *tan2 = tan1 + verticesLength;

    for (size_t i = 0; i < verticesLength; i++) {
        VROShapeVertexLayout &var = vertexLayout[i];
        
        const VROVector3f &n = { var.nx, var.ny, var.nz };
        const VROVector3f &t = tan1[i];
        
        // Gram-Schmidt orthogonalize
        VROVector3f tangent = (t - n * n.dot(t)).normalize();
        var.tx = tangent.x;
        var.ty = tangent.y;
        var.tz = tangent.z;
        
        // Calculate handedness
        var.tw = (n.cross(t).dot(tan2[i]) < 0.0F) ? -1.0F : 1.0F;
    }
    
    delete[] tan1;
}


std::vector<std::shared_ptr<VROGeometrySource>> VROShapeUtilBuildGeometrySources(std::shared_ptr<VROData> vertexData, size_t numVertices) {
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
    std::shared_ptr<VROGeometrySource> tangent = std::make_shared<VROGeometrySource>(vertexData,
                                                                                     VROGeometrySourceSemantic::Tangent,
                                                                                     numVertices,
                                                                                     true, 4,
                                                                                     sizeof(float),
                                                                                     sizeof(float) * 8,
                                                                                     sizeof(VROShapeVertexLayout));
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources = { position, texcoord, normal, tangent };
    return sources;
}
