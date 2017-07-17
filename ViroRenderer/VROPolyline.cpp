//
//  VROPolyline.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROPolyline.h"
#include "VROLog.h"
#include "VROByteBuffer.h"
#include "VROLineSegment.h"
#include "VROShapeUtils.h"
#include "VROMath.h"
#include "VROVector3f.h"
#include "VROMaterial.h"
#include "VROShaderProgram.h"
#include "VROShaderModifier.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROGeometryUtil.h"
#include "VROAnimationFloat.h"

static const int kNumJointSegments = 16;
static std::shared_ptr<VROShaderModifier> sPolylineShaderModifier;

std::shared_ptr<VROPolyline> VROPolyline::createPolyline(std::vector<VROVector3f> &path, float thickness) {
    std::vector<std::vector<VROVector3f>> paths;
    paths.push_back(path);
    return createPolyline(paths, thickness);
}

std::shared_ptr<VROPolyline> VROPolyline::createPolyline(std::vector<std::vector<VROVector3f>> &paths, float thickness) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    buildGeometry(paths, sources, elements);

    std::shared_ptr<VROPolyline> polyline = std::shared_ptr<VROPolyline>(new VROPolyline(sources, elements, thickness));

    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->getDiffuse().setColor({ 1.0, 1.0, 1.0, 1.0 });
    material->setCullMode(VROCullMode::None);

    polyline->setMaterials({ material });
    polyline->updateBoundingBox();
    return polyline;
}
void VROPolyline::buildGeometry(std::vector<std::vector<VROVector3f>> &paths,
                                std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                std::vector<std::shared_ptr<VROGeometryElement>> &elements) {

    VROByteBuffer buffer;
    size_t numVertices = 0;
    for (std::vector<VROVector3f> path : paths){
        numVertices = numVertices + encodeLine(path, buffer);
    }
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) buffer.getData(), buffer.getPosition());

    std::vector<std::shared_ptr<VROGeometrySource>> genSources = VROShapeUtilBuildGeometrySources(vertexData, numVertices);
    for (std::shared_ptr<VROGeometrySource> source : genSources) {
        sources.push_back(source);
    }

    // Each vertex is used exactly once in this strip
    int indices[numVertices];
    for (int i = 0; i < numVertices; i++) {
        indices[i] = i;
    }

    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * numVertices);
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::TriangleStrip,
                                                                                       VROGeometryUtilGetPrimitiveCount((int) numVertices, VROGeometryPrimitiveType::TriangleStrip),
                                                                                       sizeof(int));
    elements.push_back(element);
}

size_t VROPolyline::encodeLine(const std::vector<VROVector3f> path,
                               VROByteBuffer &outBuffer) {
    
    size_t numCorners = 0;
    const size_t pathSize = path.size();
    
    for (size_t i = 1; i < pathSize; i++) {
        const VROVector3f &previousCoord = path[i - 1];
        const VROVector3f &currentCoord = path[i];
        
        VROLineSegment segment({previousCoord.x, previousCoord.y, previousCoord.z}, {currentCoord.x, currentCoord.y, currentCoord.z});
        numCorners += encodeCircularEndcap(previousCoord, true, true, outBuffer);
        numCorners += encodeQuad(segment, true, true, outBuffer);
    }
    
    const VROVector3f &last = path.back();
    numCorners += encodeCircularEndcap(last, true, true, outBuffer);
    
    return numCorners;
}

size_t VROPolyline::encodeQuad(VROLineSegment segment,
                               bool beginDegenerate, bool endDegenerate, VROByteBuffer &buffer) {
    
    size_t numCorners = 4;
    if (beginDegenerate) {
        ++numCorners;
    }
    if (endDegenerate) {
        ++numCorners;
    }
    
    buffer.grow(numCorners * sizeof(VROShapeVertexLayout));
    
    VROVector3f positiveNormal = segment.normal2DUnitVector(true);
    VROVector3f negativeNormal = segment.normal2DUnitVector(false);
    
    if (beginDegenerate) {
        writeCorner(segment.getA(), negativeNormal, buffer);
    }
    
    writeCorner(segment.getA(), negativeNormal, buffer);
    writeCorner(segment.getA(), positiveNormal, buffer);
    
    writeCorner(segment.getB(), negativeNormal, buffer);
    writeCorner(segment.getB(), positiveNormal, buffer);
    
    if (endDegenerate) {
        writeCorner(segment.getB(), positiveNormal, buffer);
    }
    
    return numCorners;
}

size_t VROPolyline::encodeCircularEndcap(VROVector3f center,
                                         bool beginDegenerate, bool endDegenerate, VROByteBuffer &buffer) {
    
    float sincos[2];
    VROMathFastSinCos(2 * M_PI / kNumJointSegments, sincos);
    const float angleSin = sincos[0];
    const float angleCos = sincos[1];
    
    float x = 1;
    float y = 0;
    
    size_t numCorners = 2 * (kNumJointSegments + 1);
    if (beginDegenerate) {
        ++numCorners;
    }
    
    if (endDegenerate) {
        ++numCorners;
    }
    
    buffer.grow(numCorners * sizeof(VROShapeVertexLayout));
    
    if (beginDegenerate) {
        writeCorner(center, { x, y, 0 }, buffer);
    }
    
    for (int i = 0; i < kNumJointSegments; ++i) {
        writeCorner(center, { x, y, 0 }, buffer);
        writeCorner(center, { 0, 0, 0 }, buffer);
        
        const float temp = x;
        x = angleCos * x - angleSin * y;
        y = angleSin * temp + angleCos * y;
    }
    
    // close the circle
    writeCorner(center, { 1, 0, 0 }, buffer);
    writeCorner(center, { 0, 0, 0 }, buffer);
    
    if (endDegenerate) {
        writeCorner(center, { 0, 0, 0, }, buffer);
    }
    
    return numCorners;
}

void VROPolyline::writeCorner(VROVector3f position, VROVector3f normal, VROByteBuffer &buffer) {
    buffer.writeFloat(position.x);
    buffer.writeFloat(position.y);
    buffer.writeFloat(position.z);
    buffer.writeFloat(0); // u
    buffer.writeFloat(0); // v
    buffer.writeFloat(normal.x); // nx
    buffer.writeFloat(normal.y); // ny
    buffer.writeFloat(normal.z); // nz
    buffer.writeFloat(0); // tx
    buffer.writeFloat(0); // ty
    buffer.writeFloat(0); // tz
    buffer.writeFloat(0); // tw
}

void VROPolyline::setThickness(float thickness) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
        ((VROPolyline *)animatable)->_thickness = v;
    }, _thickness, thickness));
}

void VROPolyline::setMaterials(std::vector<std::shared_ptr<VROMaterial>> materials) {
    materials.front()->addShaderModifier(createPolylineShaderModifier());
    VROGeometry::setMaterials(materials);
}

std::shared_ptr<VROShaderModifier> VROPolyline::createPolylineShaderModifier() {
    /*
     Modifier that sets the polyline thickness.
     */
    if (!sPolylineShaderModifier) {
        std::vector<std::string> modifierCode = { "uniform float thickness;",
            "vec3 normal_offset = (thickness / 2.0) * normal;",
            "_geometry.position = _geometry.position + normal_offset;"
        };

        sPolylineShaderModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry, modifierCode);
        sPolylineShaderModifier->setUniformBinder("thickness", [](VROUniform *uniform, GLuint location, const VROGeometry &geometry) {
            const VROPolyline &polyline = dynamic_cast<const VROPolyline &>(geometry);
            uniform->setFloat(polyline.getThickness());
        });
    }
    
    return sPolylineShaderModifier;
}
