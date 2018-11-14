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
static std::shared_ptr<VROShaderModifier> sPolylineShaderGeometryModifier;
static std::shared_ptr<VROShaderModifier> sPolylineShaderVertexModifier;

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
        numCorners += encodeCircularEndcap(point, {1, 0, 0}, true, true, buffer);
    }
    else {
        VROVector3f lastPoint = getLastPoint();
        VROLineSegment segment({lastPoint.x, lastPoint.y, lastPoint.z}, {point.x, point.y, point.z});
        numCorners += encodeQuad(segment, true, true, buffer);
        numCorners += encodeCircularEndcap(point, segment.ray(), true, true, buffer);
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

    VROVector3f unit(1, 0, 0);
    VROLineSegment lastSegment({0, 0, 0}, unit);
    for (size_t i = 1; i < pathSize; i++) {
        const VROVector3f &previousCoord = path[i - 1];
        const VROVector3f &currentCoord = path[i];
        
        VROLineSegment nextSegment = VROLineSegment(previousCoord, currentCoord);
        numCorners += encodeCircularEndcap(previousCoord, lastSegment.ray(), true, true, outBuffer);
        numCorners += encodeQuad(nextSegment, true, true, outBuffer);
        
        lastSegment = nextSegment;
    }
    
    const VROVector3f &lastCoord = path.back();
    numCorners += encodeCircularEndcap(lastCoord, lastSegment.ray(), true, true, outBuffer);
    
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

size_t VROPolyline::encodeCircularEndcap(VROVector3f center, VROVector3f direction,
                                         bool beginDegenerate, bool endDegenerate, VROByteBuffer &buffer) {
    size_t numCorners = 2 * (kNumJointSegments + 1);
    if (beginDegenerate) {
        ++numCorners;
    }
    if (endDegenerate) {
        ++numCorners;
    }
    
    buffer.grow(numCorners * sizeof(VROShapeVertexLayout));
    if (beginDegenerate) {
        writeEndcapCorner(center, direction, 0, buffer);
    }
    for (int i = 0; i < kNumJointSegments; ++i) {
        // Write the endcap circle by rendering two points per rotation increment:
        // one at the center of the circle, and one at the radius of the circle. The
        // shader will perform the rotation across the appropriate axis, given the
        // angle.
        float rotation = i * 2 * M_PI / kNumJointSegments;
        writeEndcapCorner(center, direction, rotation, buffer);
        writeEndcapCorner(center, { 0, 0, 0 }, rotation, buffer);
    }
    
    // Close the circle
    writeEndcapCorner(center, direction, 2 * M_PI, buffer);
    writeEndcapCorner(center, { 0, 0, 0 }, 2 * M_PI, buffer);
    if (endDegenerate) {
        writeEndcapCorner(center, { 0, 0, 0, }, 2 * M_PI, buffer);
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
    // The shader will use the direction to compute the vector along which to
    // stroke the line (e.g. the vector across which the thickness of the line
    // spans).
    buffer.writeFloat(direction.x); // nx
    buffer.writeFloat(direction.y); // ny
    buffer.writeFloat(direction.z); // nz
    buffer.writeFloat(0); // tx
    buffer.writeFloat(0); // ty
    buffer.writeFloat(0); // tz
    buffer.writeFloat(0); // tw
}

void VROPolyline::writeEndcapCorner(VROVector3f position, VROVector3f direction, float rotation, VROByteBuffer &buffer) {
    buffer.writeFloat(position.x);
    buffer.writeFloat(position.y);
    buffer.writeFloat(position.z);
    buffer.writeFloat(0); // u
    buffer.writeFloat(0); // v
    
    // We encode the direction of the polyline (normalized) in the normal slot.
    // The shader will use the direction to compute the orientation of the endcap.
    buffer.writeFloat(direction.x); // nx
    buffer.writeFloat(direction.y); // ny
    buffer.writeFloat(direction.z); // nz
    
    // Encode the rotation in the tangent.x vector (this is an arbitrary choice).
    // The shader uses this angle of rotation to determine what triangle in the
    // endcap circle we are drawing.
    buffer.writeFloat(rotation); // tx
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
    createPolylineShaderModifiers();

    if (!materials.front()->hasShaderModifier(sPolylineShaderGeometryModifier)) {
        materials.front()->addShaderModifier(sPolylineShaderGeometryModifier);
    }
    if (!materials.front()->hasShaderModifier(sPolylineShaderVertexModifier)) {
        materials.front()->addShaderModifier(sPolylineShaderVertexModifier);
    }
    materials.front()->setCullMode(VROCullMode::None);
    VROGeometry::setMaterials(materials);
}

void VROPolyline::createPolylineShaderModifiers() {
    /*
     Modifiers that stroke a line and create circular endcaps. There is a Geometry
     and a Vertex component.
     
     For the straight segments of a line, the normal attribute is actually the
     direction of the segment. We can use that along with the camera's view vector
     to compute a plane. The normal of that plane then represents the vector along the
     width of the line (e.g. the vector along which the 'thickness' of the line is
     applied). Effectively this amounts to 'billboarding' each 2D line segment.
     
     Endcaps work similarly, except we encode a rotation value in _geometry.tangent.x.
     That rotation indicates which triangle in the circle we are currently rendering.
     We compute the width vector (stroke_offset) just as before, except this time we
     rotate it by that angle, across the the axis defined by the camera view vector.
     This effectively billboards the circular endcap (we're rotating the polyline
     direction vector about the camera view vector as we create each triangle in the
     endcap circle).
     
     Lastly, we input the *correct* normal vector at the end of this modifier to that
     lighting works as expected.
     */
    if (!sPolylineShaderGeometryModifier) {
        std::vector<std::string> geometryCode = {
            "uniform float thickness;",
            "vec4 world_position = _transforms.model_matrix * vec4(_geometry.position, 1.0);",
            "vec3 camera_ray = normalize(world_position.xyz - camera_position);",
            "vec4 line_dir = normal_matrix * vec4(_geometry.normal, 0.0);",
            "vec3 stroke_offset_dir = cross(camera_ray, line_dir.xyz);",
            
            "highp vec3 world_vertex_offset = vec3(0.0);",
            
            // Ensure we are not dealing with a 0 length vector (creates NaN when normalizing)
            "if (length(stroke_offset_dir) > 0.0) {",
            "   highp vec3 stroke_offset = normalize(stroke_offset_dir) * (thickness / 2.0);"
            "   highp float angle = _geometry.tangent.x;"
            "   if (angle > 0.0) {",
            "      highp vec3 axis = camera_ray;",
            "      highp float s = sin(angle);",
            "      highp float c = cos(angle);",
            "      highp float oc = 1.0 - c;",
            "      highp mat3 rotation = mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,",
            "                                 oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,",
            "                                 oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);",
            "      world_vertex_offset = rotation * stroke_offset;",
            "   } else {",
            "      world_vertex_offset = stroke_offset;",
            "   }",
            "}",
            // Set the normal to the inverse of the camera vector (since we're billboarding)
            "_geometry.normal = -camera_ray;"
        };

        sPolylineShaderGeometryModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry, geometryCode);
        sPolylineShaderGeometryModifier->setUniformBinder("thickness", VROShaderProperty::Float,
                                                          [](VROUniform *uniform,
                                                             const VROGeometry *geometry, const VROMaterial *material) {
            const VROPolyline *polyline = dynamic_cast<const VROPolyline *>(geometry);
            uniform->setFloat(polyline->getThickness());
        });
        sPolylineShaderGeometryModifier->setName("line_g");
        
        /*
         The Vertex component simply adds the computed vertex offset to the world position.
         */
        std::vector<std::string> vertexCode = {
            "_vertex.position = _transforms.projection_matrix * _transforms.view_matrix * (vec4(world_vertex_offset, 0.0) + world_position);",
        };
        sPolylineShaderVertexModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Vertex, vertexCode);
        sPolylineShaderVertexModifier->setName("line_v");
    }
}
