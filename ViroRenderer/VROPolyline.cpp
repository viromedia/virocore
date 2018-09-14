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

VROPolyline::VROPolyline() {
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->getDiffuse().setColor({ 1.0, 1.0, 1.0, 1.0 });
    material->setCullMode(VROCullMode::None);
    setMaterials({ material });
}

void VROPolyline::setPaths(std::vector<std::vector<VROVector3f>> &paths) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    buildGeometry(paths, sources, elements);

    setSources(sources);
    setElements(elements);
    updateBoundingBox();
}

void VROPolyline::appendPoint(VROVector3f point) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources = getGeometrySources();
    std::vector<std::shared_ptr<VROGeometryElement>> elements = getGeometryElements();
    
    // Encode the new data into a VROByteBuffer
    VROByteBuffer buffer;
    size_t numCorners = 0;
    if (isEmpty()) {
        numCorners += encodeCircularEndcap(point, true, true, buffer);
    }
    else {
        VROVector3f lastPoint = getLastPoint();
        VROLineSegment segment({lastPoint.x, lastPoint.y, lastPoint.z}, {point.x, point.y, point.z});
        numCorners += encodeQuad(segment, true, true, buffer);
        numCorners += encodeCircularEndcap(point, true, true, buffer);
    }
    
    // If there are no sources, create new ones
    if (sources.empty()) {
        std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) buffer.getData(), buffer.getPosition());
        sources = VROShapeUtilBuildGeometrySources(vertexData, numCorners);
        elements = { buildElement(numCorners) };
        
        setSources(sources);
        setElements(elements);
    }
    // Otherwise add to the existing sources
    else {
        std::vector<std::shared_ptr<VROGeometrySource>> newSources;
        std::vector<std::shared_ptr<VROGeometryElement>> newElements;
        
        std::shared_ptr<VROData> existingVertexData = sources[0]->getData();
        size_t newDataSize = existingVertexData->getDataLength() + buffer.getPosition();
        char *newData = (char *) malloc(newDataSize);
        std::shared_ptr<VROData> newVertexData = std::make_shared<VROData>(newData, newDataSize);
        
        VROByteBuffer vertexDataBuffer(newVertexData->getData(), newVertexData->getDataLength(), false);
        vertexDataBuffer.writeBytes(existingVertexData->getData(), existingVertexData->getDataLength());
        vertexDataBuffer.writeBytes(buffer.getData(), buffer.getPosition());
        
        for (std::shared_ptr<VROGeometrySource> &source : sources) {
            std::shared_ptr<VROGeometrySource> newSource = std::make_shared<VROGeometrySource>(newVertexData,
                                                                                               source->getSemantic(),
                                                                                               source->getVertexCount() + numCorners,
                                                                                               source->isFloatComponents(),
                                                                                               source->getComponentsPerVertex(),
                                                                                               source->getBytesPerComponent(),
                                                                                               source->getDataOffset(),
                                                                                               source->getDataStride());
            newSources.push_back(newSource);
        }
        
        size_t numNewVertices = sources[0]->getVertexCount() + numCorners;
        newElements = { buildElement(numNewVertices) };
        
        setSources(newSources);
        setElements(newElements);
    }
    updateBoundingBox();
}

bool VROPolyline::isEmpty() const {
    const std::vector<std::shared_ptr<VROGeometrySource>> &sources = getGeometrySources();
    if (sources.empty()) {
        return true;
    }
    
    std::shared_ptr<VROGeometrySource> position = sources[0];
    return position->getVertexCount() == 0;
}

VROVector3f VROPolyline::getLastPoint() const {
    const std::vector<std::shared_ptr<VROGeometrySource>> &sources = getGeometrySources();
    if (sources.empty()) {
        return {};
    }
    
    std::shared_ptr<VROGeometrySource> vertexSource = sources[0];
    if (vertexSource->getVertexCount() == 0) {
        return {};
    }
    
    std::shared_ptr<VROData> data = vertexSource->getData();
    int lastPosition = (vertexSource->getVertexCount() - 1) * vertexSource->getDataStride();
    
    VROByteBuffer buffer((char *)data->getData() + lastPosition, vertexSource->getDataStride());
    VROVector3f point(buffer.readFloat(), buffer.readFloat(), buffer.readFloat());
    return point;
}

void VROPolyline::buildGeometry(std::vector<std::vector<VROVector3f>> &paths,
                                std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                std::vector<std::shared_ptr<VROGeometryElement>> &elements) {

    VROByteBuffer buffer;
    size_t numVertices = 0;
    for (std::vector<VROVector3f> &path : paths) {
        if (!path.empty()) {
            numVertices = numVertices + encodeLine(path, buffer);
        }
    }
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) buffer.getData(), buffer.getPosition());

    std::vector<std::shared_ptr<VROGeometrySource>> genSources = VROShapeUtilBuildGeometrySources(vertexData, numVertices);
    for (std::shared_ptr<VROGeometrySource> source : genSources) {
        sources.push_back(source);
    }

    elements.push_back(buildElement(numVertices));
}

std::shared_ptr<VROGeometryElement> VROPolyline::buildElement(size_t numCorners) {
    int indices[numCorners];
    for (int i = 0; i < numCorners; i++) {
        indices[i] = i;
    }
    
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * numCorners);
    std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                       VROGeometryPrimitiveType::TriangleStrip,
                                                                                       VROGeometryUtilGetPrimitiveCount((int) numCorners, VROGeometryPrimitiveType::TriangleStrip),
                                                                                       sizeof(int));
    return element;
}

size_t VROPolyline::encodeLine(const std::vector<VROVector3f> &path,
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
    
    VROVector3f positiveDirection = segment.ray();
    VROVector3f negativeDirection = positiveDirection.scale(-1);

    if (beginDegenerate) {
        writeCorner(segment.getA(), negativeDirection, buffer);
    }
    
    writeCorner(segment.getA(), negativeDirection, buffer);
    writeCorner(segment.getA(), positiveDirection, buffer);
    
    writeCorner(segment.getB(), negativeDirection, buffer);
    writeCorner(segment.getB(), positiveDirection, buffer);
    
    if (endDegenerate) {
        writeCorner(segment.getB(), positiveDirection, buffer);
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

void VROPolyline::writeCorner(VROVector3f position, VROVector3f direction, VROByteBuffer &buffer) {
    buffer.writeFloat(position.x);
    buffer.writeFloat(position.y);
    buffer.writeFloat(position.z);
    buffer.writeFloat(0); // u
    buffer.writeFloat(0); // v

    // We encode the direction of the polyline (normalized) in the normal slot.
    // The shader will use the direction to compute the thickness of the line
    buffer.writeFloat(direction.x); // nx
    buffer.writeFloat(direction.y); // ny
    buffer.writeFloat(direction.z); // nz
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
    createPolylineShaderModifier();

    if (!materials.front()->hasShaderModifier(sPolylineShaderModifier)) {
        materials.front()->addShaderModifier(sPolylineShaderModifier);
    }
    materials.front()->setCullMode(VROCullMode::None);
    VROGeometry::setMaterials(materials);
}

std::shared_ptr<VROShaderModifier> VROPolyline::createPolylineShaderModifier() {
    /*
     Modifier that sets the polyline thickness. Our normal attribute here is actually the
     direction of the polyline segment. We can use that along with the camera's view vector
     to compute a plane. The line's offset is then either the positive or negative normal
     vector of the plane.
     */
    if (!sPolylineShaderModifier) {
        std::vector<std::string> modifierCode = { "uniform float thickness;",
            "vec3 world_pos = (_transforms.model_matrix * vec4(_geometry.position, 1.0)).xyz;",
            "vec3 camera_ray = normalize(world_pos - camera_position);",
            "vec3 line_normal = cross(camera_ray, _geometry.normal);",
            "vec3 normal_offset = (thickness / 2.0) * line_normal;",
            "_geometry.position = _geometry.position + normal_offset;"
        };

        sPolylineShaderModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry, modifierCode);
        sPolylineShaderModifier->setUniformBinder("thickness", [](VROUniform *uniform, GLuint location,
                                                                  const VROGeometry *geometry, const VROMaterial *material) {
            const VROPolyline *polyline = dynamic_cast<const VROPolyline *>(geometry);
            uniform->setFloat(polyline->getThickness());
        });
        sPolylineShaderModifier->setName("line");
    }
    
    return sPolylineShaderModifier;
}
