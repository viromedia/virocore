//
//  VROPolygon.cpp
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROPolygon.h"
#include "VROData.h"
#include "VROGeometryUtil.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROMaterial.h"
#include "VROLog.h"
#include "stdlib.h"
#include "VROByteBuffer.h"

std::shared_ptr<VROPolygon> VROPolygon::createPolygon(std::vector<VROVector3f> boundaryVertices,
                                                      float u0, float v0, float u1, float v1) {
    std::shared_ptr<VROPolygon> surface = std::shared_ptr<VROPolygon>(new VROPolygon(boundaryVertices, u0, v0, u1, v1));
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    surface->setMaterials({ material });
    return surface;
}

VROPolygon::VROPolygon(std::vector<VROVector3f> boundaries,
                       float u0, float v0, float u1, float v1) :
    _boundaryVertices(boundaries),
    _u0(u0),
    _v0(v0),
    _u1(u1),
    _v1(v1) {

    // Determine the min and max vertex coordinates for this polygon.
    _minX = FLT_MAX;
    _maxX = -FLT_MAX;
    _minY = FLT_MAX;
    _maxY = -FLT_MAX;
    for (VROVector3f vec : _boundaryVertices) {
        _minX = std::min(_minX, vec.x);
        _maxX = std::max(_maxX, vec.x);
        _minY = std::min(_minY, vec.y);
        _maxY = std::max(_maxY, vec.y);
    }

    updateSurface();
}

VROPolygon::~VROPolygon() {
}

void VROPolygon::updateSurface() {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;

    // Build the polygon here to get sources and elements
    passert(_boundaryVertices.size() > 0);
    buildGeometry(_boundaryVertices, sources, elements);

    // Then set the geometric elements and sources that we've just built.
    setSources(sources);
    setElements(elements);
    updateBoundingBox();
}

void VROPolygon::buildGeometry(std::vector<VROVector3f> boundaryPath,
                                     std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                     std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    // Start creating our buffer of vertex indices that references each corner within this polygon.
    VROByteBuffer buffer;
    for (VROVector3f &point : boundaryPath) {
        buffer.grow(sizeof(VROShapeVertexLayout));
        writePolygonCorner(point, buffer);
    }
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) buffer.getData(), buffer.getPosition());

    // Parse our constructed buffered vertex data into actual usable geometry sources
    // with (normal, tang, vertex, uvs).
    size_t numVertices = boundaryPath.size();
    std::vector<std::shared_ptr<VROGeometrySource>> genSources = VROShapeUtilBuildGeometrySources(vertexData, numVertices);
    for (std::shared_ptr<VROGeometrySource> source : genSources) {
        sources.push_back(source);
    }

    // Reference our index-ed buffered data above and build an array representing the
    // vertex path upon which the renderer will transverse to render our polygon.
    elements.push_back(buildElement(numVertices));
}

std::shared_ptr<VROGeometryElement> VROPolygon::buildElement(size_t pathLength) {
    int indices[pathLength];

    // The starting vertex.
    int pathIndex = 0;
    indices[pathIndex] = 0;

    /*
     Alternate back and forth between our first and last polygon corners towards the middle.
     This will ensure that triangles will properly fill up the entirety of the polygon shape.
     And more importantly, that the winding order used when rendering Triangle strips is
     consistent, ensuring consistent normals and as a result a tessellated polygon. This
     should handle both odd and even sided polygon shapes.
     */
    for (int i = 1; i <= pathLength/2; i++) {
        pathIndex++;
        indices[pathIndex] = (int) (pathLength - i);

        if (pathLength - i == i) {
            continue;
        }

        pathIndex++;
        indices[pathIndex] = i;
    }

    // Finally, consolidate our indexed vertex path data into VROGeometryElement to be rendered.
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * pathLength);
    std::shared_ptr<VROGeometryElement> element
            = std::make_shared<VROGeometryElement>(indexData,
                                                   VROGeometryPrimitiveType::TriangleStrip,
                                                   VROGeometryUtilGetPrimitiveCount((int) pathLength,
                                                                                    VROGeometryPrimitiveType::TriangleStrip),
                                                   sizeof(int));
    return element;
}

void VROPolygon::writePolygonCorner(VROVector3f position, VROByteBuffer &buffer) {
    float u = (position.x - _minX) / (_maxX - _minX) * (_u1 - _u0);
    float v = (position.y - _maxY) / (_minY - _maxY) * (_v1 - _v0);
    buffer.writeFloat(position.x);
    buffer.writeFloat(position.y);
    buffer.writeFloat(0);
    buffer.writeFloat(u);
    buffer.writeFloat(v);
    buffer.writeFloat(0); // nx
    buffer.writeFloat(0); // ny
    buffer.writeFloat(1); // nz
    buffer.writeFloat(0); // tx
    buffer.writeFloat(0); // ty
    buffer.writeFloat(0); // tz
    buffer.writeFloat(0); // tw
}
