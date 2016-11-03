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
#include "VROLog.h"
#include "VROImageUIKit.h"

std::vector<std::shared_ptr<VRONode>> VROLoader::loadURL(NSURL *url) {
    std::vector<std::shared_ptr<VRONode>> results;
    
    MDLAsset *asset = [[MDLAsset alloc] initWithURL:url];
    for (int i = 0; i < asset.count; i++) {
        MDLObject *object = [asset objectAtIndex:i];
        
        if ([object isKindOfClass:[MDLMesh class]]) {
            results.push_back(loadMesh((MDLMesh *) object));
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

std::shared_ptr<VRONode> VROLoader::loadMesh(MDLMesh *mesh) {
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
            
            std::pair<bool, int> bytesPerComponent = getBytesPerComponent(attribute.format);
            int componentsPerVertex = getComponentsPerVertex(attribute.format);
            
            std::shared_ptr<VROGeometrySource> source = std::make_shared<VROGeometrySource>(data,
                                                                                            semantic,
                                                                                            mesh.vertexCount,
                                                                                            bytesPerComponent.first,
                                                                                            componentsPerVertex,
                                                                                            bytesPerComponent.second,
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
            
            std::pair<bool, int> bytesPerComponent = getBytesPerComponent(attribute.format);
            int componentsPerVertex = getComponentsPerVertex(attribute.format);
            
            std::shared_ptr<VROGeometrySource> source = std::make_shared<VROGeometrySource>(data,
                                                                                            semantic,
                                                                                            mesh.vertexCount,
                                                                                            bytesPerComponent.first,
                                                                                            componentsPerVertex,
                                                                                            bytesPerComponent.second,
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
                                                                                           getPrimitiveCount((int) submesh.indexCount, primitive),
                                                                                           bytesPerIndex);
        elements.push_back(element);
    }
    
    std::shared_ptr<VROGeometry> geometry = std::make_shared<VROGeometry>(sources, elements);
    
    for (MDLSubmesh *submesh : [mesh submeshes]) {
        MDLMaterial *material = submesh.material;
        
        std::shared_ptr<VROMaterial> vM = std::make_shared<VROMaterial>();
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
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
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

std::pair<bool, int> VROLoader::getBytesPerComponent(MDLVertexFormat format) {
    int bitsValue = (format & 0xFFF00);
    switch (bitsValue) {
        case MDLVertexFormatCharBits:
            return std::pair<bool, int>(false, 1);
            
        case MDLVertexFormatShortBits:
        case MDLVertexFormatUShortBits:
            return std::pair<bool, int>(false, 2);
            
        case MDLVertexFormatHalfBits:
            return std::pair<bool, int>(true, 2);;
            
        case MDLVertexFormatIntBits:
        case MDLVertexFormatUIntBits:
            return std::pair<bool, int>(false, 4);
            
        case MDLVertexFormatFloatBits:
            return std::pair<bool, int>(true, 4);
            
        default:
            pabort();
    }
}

int VROLoader::getComponentsPerVertex(MDLVertexFormat format) {
    return (format & 0xFF);
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
    return std::make_shared<VROTexture>(std::make_shared<VROImageUIKit>(image));
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
