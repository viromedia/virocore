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

static const int kNumJointSegments = 16;

std::shared_ptr<VROPolyline> VROPolyline::createPolyline(std::vector<VROVector3f> &path, float width) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    buildGeometry(path, sources, elements);
    
    std::shared_ptr<VROPolyline> polyline = std::shared_ptr<VROPolyline>(new VROPolyline(sources, elements));
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setWritesToDepthBuffer(false);
    material->setReadsFromDepthBuffer(false);
    material->getDiffuse().setContents({ 1.0, 1.0, 1.0, 1.0 });
    material->setCullMode(VROCullMode::None);
    material->setLightingModel(VROLightingModel::Constant);
    
    std::vector<std::string> modifierCode = { "uniform float width;",
                                              "vec3 normal_offset = (width / 2.0) * normal;",
                                              "_geometry.position = _geometry.position + normal_offset;"
                                            };
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                                      modifierCode);
    modifier->setUniformBinder("width", [width](VROUniform *uniform, GLuint location) {
        uniform->setFloat(width);
    });
    
    material->addShaderModifier(modifier);
    polyline->getMaterials().push_back(material);
    return polyline;
}

void VROPolyline::buildGeometry(std::vector<VROVector3f> &path,
                                std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    VROByteBuffer buffer;
    size_t numVertices = encodeLine(path, buffer);
    
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) buffer.getData(), buffer.getPosition());
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
        
        VROLineSegment segment({previousCoord.x, previousCoord.y}, {currentCoord.x, currentCoord.y});
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
}