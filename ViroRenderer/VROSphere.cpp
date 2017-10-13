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
    return std::make_shared<VROSphere>(radius, widthSegments, heightSegments, facesOutward);
}

VROSphere::VROSphere(float radius, int widthSegments, int heightSegments, bool facesOutward) :
    _radius(radius),
    _widthSegments(widthSegments),
    _heightSegments(heightSegments),
    _facesOutward(facesOutward) {

    updateSphere();
}

void VROSphere::setRadius(float radius) {
    _radius = radius;
    updateSphere();
}

void VROSphere::setWidthSegments(int widthSegments) {
    _widthSegments = widthSegments;
    updateSphere();
}

void VROSphere::setHeightSegments(int heightSegments) {
    _heightSegments = heightSegments;
    updateSphere();
}

void VROSphere::setFacesOutward(bool facesOutward) {
    _facesOutward = facesOutward;
    updateSphere();
}

void VROSphere::updateSphere() {
    // If there are no materials, set a default one
    if (getMaterials().empty()) {
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        VROGeometry::setMaterials({ material });
    }

    float phiStart = 0;
    float phiLength = M_PI * 2.0;

    float thetaStart = 0;
    float thetaLength = M_PI;
    float thetaEnd = thetaStart + thetaLength;

    int vertexCount = ((_widthSegments + 1) * (_heightSegments + 1));
    int varSizeBytes = sizeof(VROShapeVertexLayout) * vertexCount;

    // Will be moved to VROData so does not need to be explicitly freed!
    VROShapeVertexLayout *var = (VROShapeVertexLayout *)malloc(varSizeBytes);

    int index = 0;
    std::vector<std::vector<int>> vertices;

    for (int y = 0; y <= _heightSegments; y++) {
        std::vector<int> verticesRow;
        float v = (float) y / (float) _heightSegments;

        for (int x = 0; x <= _widthSegments; x++) {
            float u = (float) x / (float) _widthSegments;

            float px = -_radius * cos(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);
            float py =  _radius * cos(thetaStart + v * thetaLength);
            float pz =  _radius * sin(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);

            VROVector3f normal(px, py, pz);
            normal = normal.normalize();

            if (!_facesOutward) {
                normal = normal.scale(-1);
            }

            passert (index < varSizeBytes / sizeof(VROShapeVertexLayout));

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

    for (int y = 0; y < _heightSegments; y++) {
        for (int x = 0; x < _widthSegments; x++) {
            int v1 = vertices[y][x + 1];
            int v2 = vertices[y][x];
            int v3 = vertices[y + 1][x];
            int v4 = vertices[y + 1][x + 1];

            if (!_facesOutward) {
                if (y != 0 || thetaStart > 0) {
                    indices.push_back(v1);
                    indices.push_back(v4);
                    indices.push_back(v2);
                }
                if (y != _heightSegments - 1 || thetaEnd < M_PI) {
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
                if (y != _heightSegments - 1 || thetaEnd < M_PI) {
                    indices.push_back(v2);
                    indices.push_back(v3);
                    indices.push_back(v4);
                }
            }
        }
    }

    VROShapeUtilComputeTangents(var, vertexCount, indices.data(), indices.size());

    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) var, varSizeBytes, VRODataOwnership::Move);
    std::vector<std::shared_ptr<VROGeometrySource>> sources = VROShapeUtilBuildGeometrySources(vertexData, vertexCount);
    setSources(sources);

    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices.data(), sizeof(int) * indices.size());
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::Triangle,
                                                                                       indices.size() / 3,
                                                                                       sizeof(int));
    std::vector<std::shared_ptr<VROGeometryElement>> elements = { element };
    setElements(elements);
    updateBoundingBox();
}
