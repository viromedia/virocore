//
//  VROLoader.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROLoader.h"
#include "VROGeometry.h"
#include "VROMaterial.h"
#include "VROTexture.h"
#include "VROData.h"
#include "VROMaterialVisual.h"
#include "VRONode.h"

std::vector<std::shared_ptr<VRONode>> VROLoader::loadURL(NSURL *url,
                                                         const VRORenderContext &context) {
    std::vector<std::shared_ptr<VRONode>> results;
    
    MDLAsset *asset = [[MDLAsset alloc] initWithURL:url];
    for (int i = 0; i < asset.count; i++) {
        MDLObject *object = [asset objectAtIndex:i];
        
        if ([object isKindOfClass:[MDLMesh class]]) {
            results.push_back(loadMesh((MDLMesh *) object, context));
        }
        else if ([object isKindOfClass:[MDLLight class]]) {
            // TODO
        }
        else if ([object isKindOfClass:[MDLCamera class]]) {
            // TODO
        }
    }
    
    return results;
}

std::shared_ptr<VRONode> VROLoader::loadMesh(MDLMesh *mesh,
                                             const VRORenderContext &context) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    
    MDLVertexDescriptor *descriptor = mesh.vertexDescriptor;
    NSArray <MDLVertexAttribute *> *attributes = descriptor.attributes;
    
    NSArray<id<MDLMeshBuffer>> *vertexBuffers = mesh.vertexBuffers;
    
    // Array of structures design
    if (vertexBuffers.count == 1) {
        id <MDLMeshBuffer> buffer = [vertexBuffers objectAtIndex:0];
        MDLVertexBufferLayout *layout = [descriptor.layouts objectAtIndex:0];

        std::shared_ptr<VROData> data = std::make_shared<VROData>([buffer map].bytes, buffer.length);
        
        for (MDLVertexAttribute *attribute in attributes) {
            VROGeometrySourceSemantic semantic = parseSemantic(attribute.name);
            if (semantic == VROGeometrySourceSemantic::Invalid) {
                continue;
            }
            
            std::pair<int, int> format = parseFormat(attribute.format);
            
            std::shared_ptr<VROGeometrySource> source = std::make_shared<VROGeometrySource>(data,
                                                                                            semantic,
                                                                                            mesh.vertexCount,
                                                                                            format.first,
                                                                                            format.second,
                                                                                            attribute.offset,
                                                                                            layout.stride);
            sources.push_back(source);
        }
    }
    
    // Structure of arrays design
    else {
        NSArray <id <MDLMeshBuffer>> *buffers = [mesh vertexBuffers];
        NSArray <MDLVertexBufferLayout *> *layouts = descriptor.layouts;

        for (int i = 0; i < buffers.count; i++) {
            id <MDLMeshBuffer> buffer = [buffers objectAtIndex:i];
            MDLVertexBufferLayout *layout = [layouts objectAtIndex:i];
            MDLVertexAttribute *attribute = [attributes objectAtIndex:i];
            
            std::shared_ptr<VROData> data = std::make_shared<VROData>([buffer map].bytes, buffer.length);
            
            VROGeometrySourceSemantic semantic = parseSemantic(attribute.name);
            std::pair<int, int> format = parseFormat(attribute.format);
            
            std::shared_ptr<VROGeometrySource> source = std::make_shared<VROGeometrySource>(data,
                                                                                            semantic,
                                                                                            mesh.vertexCount,
                                                                                            format.first,
                                                                                            format.second,
                                                                                            attribute.offset,
                                                                                            layout.stride);
            sources.push_back(source);
        }
    }
    
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    
    for (MDLSubmesh *submesh : [mesh submeshes]) {
        id <MDLMeshBuffer> buffer = submesh.indexBuffer;
        std::shared_ptr<VROData> data = std::make_shared<VROData>([buffer map].bytes, buffer.length);
        
        int bytesPerIndex = parseIndexSize(submesh.indexType);
        VROGeometryPrimitiveType primitive = parsePrimitive(submesh.geometryType);
        
        std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(data,
                                                                                           primitive,
                                                                                           getPrimitiveCount(submesh.indexCount, primitive),
                                                                                           bytesPerIndex);
        elements.push_back(element);
    }
    
    std::shared_ptr<VROGeometry> geometry = std::make_shared<VROGeometry>(sources, elements);
    
    for (MDLSubmesh *submesh : [mesh submeshes]) {
        MDLMaterial *material = submesh.material;
        
        std::shared_ptr<VROMaterial> vM = std::make_shared<VROMaterial>();
        vM->setLightingModel(VROLightingModel::Blinn);
        vM->setReadsFromDepthBuffer(true);
        vM->setWritesToDepthBuffer(true);
        
        for (int i = 0; i < material.count; i++) {
            MDLMaterialProperty *property = [material objectAtIndexedSubscript:i];
            MDLMaterialSemantic semantic = property.semantic;
            
            if (semantic == MDLMaterialSemanticBaseColor) {
                if (property.type == MDLMaterialPropertyTypeColor) {
                    vM->getDiffuse().setContents(parseColor([property color]));
                }
                else if (property.type == MDLMaterialPropertyTypeTexture) {
                    vM->getDiffuse().setContents(parseTexture([property textureSamplerValue]));
                }
            }
            
            if (semantic == MDLMaterialSemanticSpecular) {
                if (property.type == MDLMaterialPropertyTypeColor) {
                    vM->getSpecular().setContents(parseColor([property color]));
                }
                else if (property.type == MDLMaterialPropertyTypeTexture) {
                    vM->getSpecular().setContents(parseTexture([property textureSamplerValue]));
                }
            }
        }
        
        geometry->getMaterials().push_back(vM);
    }
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>(context);
    node->setGeometry(geometry);
    
    return node;
}

VROGeometrySourceSemantic VROLoader::parseSemantic(NSString *string) {
    if ([string isEqualToString:MDLVertexAttributePosition]) {
        return VROGeometrySourceSemantic::Vertex;
    }
    if ([string isEqualToString:MDLVertexAttributeNormal]) {
        return VROGeometrySourceSemantic::Normal;
    }
    if ([string isEqualToString:MDLVertexAttributeColor]) {
        return VROGeometrySourceSemantic::Color;
    }
    if ([string isEqualToString:MDLVertexAttributeTextureCoordinate]) {
        return VROGeometrySourceSemantic::Texcoord;
    }
    return VROGeometrySourceSemantic::Invalid;
}

std::pair<int, int> VROLoader::parseFormat(MDLVertexFormat format) {
    switch (format) {
        case MDLVertexFormatChar:
        case MDLVertexFormatUChar:
            return std::pair<int, int>(1, 1);
            
        case MDLVertexFormatChar2:
        case MDLVertexFormatUChar2:
            return std::pair<int, int>(2, 1);
            
        case MDLVertexFormatChar3:
        case MDLVertexFormatUChar3:
            return std::pair<int, int>(3, 1);
            
        case MDLVertexFormatChar4:
        case MDLVertexFormatUChar4:
            return std::pair<int, int>(4, 1);
            
        case MDLVertexFormatShort:
        case MDLVertexFormatUShort:
        case MDLVertexFormatHalf:
            return std::pair<int, int>(1, 2);
            
        case MDLVertexFormatShort2:
        case MDLVertexFormatUShort2:
        case MDLVertexFormatHalf2:
            return std::pair<int, int>(2, 2);
            
        case MDLVertexFormatShort3:
        case MDLVertexFormatUShort3:
        case MDLVertexFormatHalf3:
            return std::pair<int, int>(3, 2);
            
        case MDLVertexFormatShort4:
        case MDLVertexFormatUShort4:
        case MDLVertexFormatHalf4:
            return std::pair<int, int>(4, 2);
            
        case MDLVertexFormatInt:
        case MDLVertexFormatUInt:
        case MDLVertexFormatFloat:
            return std::pair<int, int>(1, 4);
            
        case MDLVertexFormatInt2:
        case MDLVertexFormatUInt2:
        case MDLVertexFormatFloat2:
            return std::pair<int, int>(2, 4);
            
        case MDLVertexFormatInt3:
        case MDLVertexFormatUInt3:
        case MDLVertexFormatFloat3:
            return std::pair<int, int>(3, 4);
            
        case MDLVertexFormatInt4:
        case MDLVertexFormatUInt4:
        case MDLVertexFormatFloat4:
            return std::pair<int, int>(4, 4);
        
        default:
            return std::pair<int, int>(0, 0);
    }
}

int VROLoader::parseIndexSize(MDLIndexBitDepth depth) {
    switch (depth) {
        case MDLIndexBitDepthInvalid:
            return 0;
        case MDLIndexBitDepthUInt8:
            return 1;
        case MDLIndexBitDepthUint16:
            return 2;
        case MDLIndexBitDepthUInt32:
            return 4;
    }
    
    return 4;
}

VROGeometryPrimitiveType VROLoader::parsePrimitive(MDLGeometryType type) {
    switch (type) {
        case MDLGeometryTypePoints:
            return VROGeometryPrimitiveType::Point;
        case MDLGeometryTypeLines:
            return VROGeometryPrimitiveType::Line;
        case MDLGeometryTypeTriangles:
            return VROGeometryPrimitiveType::Triangle;
        case MDLGeometryTypeTriangleStrips:
            return VROGeometryPrimitiveType::TriangleStrip;
        default:
            return VROGeometryPrimitiveType::Triangle;
    }
}

VROVector4f VROLoader::parseColor(CGColorRef colorRef) {
    const CGFloat *components = CGColorGetComponents(colorRef);
    VROVector4f color(components[0], components[1], components[2], components[3]);
    
    return color;
}


std::shared_ptr<VROTexture> VROLoader::parseTexture(MDLTextureSampler *sampler) {
    UIImage *image = [UIImage imageWithCGImage:[sampler.texture imageFromTexture]];
    return std::make_shared<VROTexture>(image);
}

int VROLoader::getPrimitiveCount(int indexCount, VROGeometryPrimitiveType primitiveType) {
    switch (primitiveType) {
        case VROGeometryPrimitiveType::Triangle:
            return indexCount / 3;
            
        case VROGeometryPrimitiveType::TriangleStrip:
            return indexCount - 2;
            
        case VROGeometryPrimitiveType::Line:
            return indexCount / 2;
            
        case VROGeometryPrimitiveType::Point:
            return indexCount;
            
        default:
            break;
    }
}
