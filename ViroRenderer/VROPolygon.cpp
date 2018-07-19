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
#include "poly2tri/poly2tri.h"

std::shared_ptr<VROPolygon> VROPolygon::createPolygon(std::vector<VROVector3f> path,
                                                      float u0, float v0, float u1, float v1) {
    std::vector<std::vector<VROVector3f>> holes;
    return createPolygon(path, holes, u0, v0, u1, v1);
}

std::shared_ptr<VROPolygon> VROPolygon::createPolygon(std::vector<VROVector3f> path,
                                                      std::vector<std::vector<VROVector3f>> holes,
                                                      float u0, float v0, float u1, float v1) {
    std::shared_ptr<VROPolygon> surface = std::shared_ptr<VROPolygon>(new VROPolygon(path, holes, u0, v0, u1, v1));
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    surface->setMaterials({ material });
    return surface;
}

VROPolygon::VROPolygon(std::vector<VROVector3f> path, std::vector<std::vector<VROVector3f>> holes,
                       float u0, float v0, float u1, float v1) :
    _path(path),
    _holes(holes),
    _u0(u0),
    _v0(v0),
    _u1(u1),
    _v1(v1) {

    // Determine the min and max vertex coordinates for this polygon.
    _minX = FLT_MAX;
    _maxX = -FLT_MAX;
    _minY = FLT_MAX;
    _maxY = -FLT_MAX;
    for (VROVector3f vec : _path) {
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
    passert(_path.size() > 0);
    buildGeometry(sources, elements);

    // Then set the geometric elements and sources that we've just built.
    setSources(sources);
    setElements(elements);
    updateBoundingBox();
}

void VROPolygon::buildGeometry(std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                               std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    const std::size_t pathSize = _path.size();
    
    /*
     Convert to poly2tri data structures.
     */
    std::vector<p2t::Point *> p2tPath;
    p2tPath.reserve(pathSize);

    for (VROVector3f &v : _path) {
        p2t::Point *p = new p2t::Point(v.x, v.y);
        p2tPath.push_back(p);
    }
    p2t::CDT cdt(p2tPath);
    
    std::vector<std::vector<p2t::Point *>> p2tHoles;
    p2tHoles.reserve(_holes.size());
    
    for (std::vector<VROVector3f> &hole : _holes) {
        std::vector<p2t::Point *> p2tHole;
        p2tHole.reserve(hole.size());
        
        for (VROVector3f &v : hole) {
            p2t::Point *p = new p2t::Point(v.x, v.y);
            p2tHole.push_back(p);
        }
        
        cdt.AddHole(p2tHole);
        p2tHoles.push_back(std::move(p2tHole));
    }
    
    // Triangulate
    cdt.Triangulate();
    std::vector<p2t::Triangle *> triangles = cdt.GetTriangles();
    
    // Start creating our buffer of vertex indices that references each corner within this polygon.
    VROByteBuffer buffer;
    for (p2t::Triangle *triangle : triangles) {
        p2t::Point *a = triangle->GetPoint(0);
        p2t::Point *b = triangle->GetPoint(1);
        p2t::Point *c = triangle->GetPoint(2);
        
        buffer.grow(sizeof(VROShapeVertexLayout) * 3);
        writePolygonCorner(a, buffer);
        writePolygonCorner(b, buffer);
        writePolygonCorner(c, buffer);
    }
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) buffer.getData(), buffer.getPosition());
    
    // Parse our constructed buffered vertex data into actual usable geometry sources
    // with (normal, tang, vertex, uvs).
    size_t numCorners = triangles.size() * 3;
    std::vector<std::shared_ptr<VROGeometrySource>> genSources = VROShapeUtilBuildGeometrySources(vertexData, numCorners);
    for (std::shared_ptr<VROGeometrySource> source : genSources) {
        sources.push_back(source);
    }
    elements.push_back(buildElement(numCorners));
    
    // Cleanup poly2tri structures
    for (auto p : p2tPath) {
        delete (p);
    }
    p2tPath.clear();
    
    for (auto &hole : p2tHoles) {
        for (auto p : hole) {
            delete (p);
        }
        hole.clear();
    }
    p2tHoles.clear();
}

std::shared_ptr<VROGeometryElement> VROPolygon::buildElement(size_t numCorners) {
    int indices[numCorners];
    for (int i = 0; i < numCorners; i++) {
        indices[i] = i;
    }

    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * numCorners);
    std::shared_ptr<VROGeometryElement> element= std::make_shared<VROGeometryElement>(indexData,
                                                                                      VROGeometryPrimitiveType::Triangle,
                                                                                      VROGeometryUtilGetPrimitiveCount((int) numCorners,
                                                                                                                       VROGeometryPrimitiveType::Triangle),
                                                                                      sizeof(int));
    return element;
}

void VROPolygon::writePolygonCorner(p2t::Point *position, VROByteBuffer &buffer) {
    float u = _u0 + (position->x - _minX) / (_maxX - _minX) * (_u1 - _u0);
    float v = _v0 + (position->y - _maxY) / (_minY - _maxY) * (_v1 - _v0);
    buffer.writeFloat(position->x);
    buffer.writeFloat(position->y);
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
