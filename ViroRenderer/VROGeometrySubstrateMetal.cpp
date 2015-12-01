//
//  VROGeometrySubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROGeometrySubstrateMetal.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VRORenderContextMetal.h"
#include "VROLog.h"
#include <map>

VROGeometrySubstrateMetal::VROGeometrySubstrateMetal(const VRORenderContextMetal &context,
                                                     std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                                     std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    id <MTLDevice> device = context.getDevice();

    readGeometryElements(device, elements);
    readGeometrySources(device, sources);
}

VROGeometrySubstrateMetal::~VROGeometrySubstrateMetal() {
    
}

void VROGeometrySubstrateMetal::readGeometryElements(id <MTLDevice> device,
                                                     std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    for (std::shared_ptr<VROGeometryElement> element : elements) {
        VROGeometryElementMetal elementMetal;
        
        elementMetal.buffer = [device newBufferWithBytesNoCopy:element->getData()->getData()
                                                        length:element->getPrimitiveCount() * element->getBytesPerIndex()
                                                       options:0 deallocator:nullptr];
        elementMetal.primitiveType = parsePrimitiveType(element->getPrimitiveType());
        elementMetal.indexCount = element->getPrimitiveCount();
        elementMetal.indexType = (element->getBytesPerIndex() == 2) ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
        elementMetal.indexBufferOffset = 0;
        
        _elements.push_back(elementMetal);
    }
}

void VROGeometrySubstrateMetal::readGeometrySources(id <MTLDevice> device,
                                                    std::vector<std::shared_ptr<VROGeometrySource>> &sources) {
        
    std::shared_ptr<VROGeometrySource> source = sources.front();
    std::map<std::shared_ptr<VROData>, std::vector<std::shared_ptr<VROGeometrySource>>> dataMap;
    
    /*
     Sort the sources into groups defined by the data array they're using.
     */
    for (std::shared_ptr<VROGeometrySource> source : sources) {
        std::shared_ptr<VROData> data = source->getData();
        
        auto it = dataMap.find(data);
        if (it == dataMap.end()) {
            std::vector<std::shared_ptr<VROGeometrySource>> group = { source };
            dataMap[data] = group;
        }
        else {
            std::vector<std::shared_ptr<VROGeometrySource>> &group = it->second;
            group.push_back(source);
        }
    }
    
    /*
     For each group, create an MTLBuffer and an MTLVertexDescriptor.
     */
    for (auto &kv : dataMap) {
        VROGeometrySourceMetal metalDesc;
        std::vector<std::shared_ptr<VROGeometrySource>> group = kv.second;
        
        /*
         Create a metal buffer that wraps over the VROData.
         */
        int dataSize = 0;
        for (std::shared_ptr<VROGeometrySource> source : group) {
            int size = source->getVertexCount() * source->getComponentsPerVertex() *
            source->getBytesPerComponent();
            
            dataSize = std::max(dataSize, size);
        }
        
        metalDesc.buffer = [device newBufferWithBytesNoCopy:kv.first->getData()
                                                     length:dataSize options:0
                                                deallocator:nullptr];
        metalDesc.buffer.label = @"VROGeometryVertexBuffer";
        
        /*
         Create the vertex descriptor for all sources interleaved over this data.
         */
        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.layouts[0].stepRate = 1;
        vertexDescriptor.layouts[0].stride = group[0]->getDataStride();
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        
        for (int i = 0; i < group.size(); i++) {
            std::shared_ptr<VROGeometrySource> source = group[i];
            int attrIdx = parseAttributeIndex(source->getSemantic());
            
            vertexDescriptor.attributes[attrIdx].format = parseVertexFormat(source);
            vertexDescriptor.attributes[attrIdx].offset = source->getDataOffset();
            vertexDescriptor.attributes[attrIdx].bufferIndex = 0;
            
            passert (source->getDataStride() == vertexDescriptor.layouts[0].stride);
        }
        
        metalDesc.descriptor = vertexDescriptor;
        _sources.push_back(metalDesc);
    }
}

MTLVertexFormat VROGeometrySubstrateMetal::parseVertexFormat(std::shared_ptr<VROGeometrySource> &source) {
    // Currently assuming floats
    switch (source->getBytesPerComponent()) {
        case 2:
            switch (source->getComponentsPerVertex()) {
                case 1:
                    return MTLVertexFormatFloat;
                    
                case 2:
                    return MTLVertexFormatFloat2;
                    
                case 3:
                    return MTLVertexFormatFloat3;
                    
                case 4:
                    return MTLVertexFormatFloat4;
                    
                default:
                    pabort();
                    return MTLVertexFormatFloat;
            }
            
        case 4:
            switch (source->getComponentsPerVertex()) {
                case 1:
                    return MTLVertexFormatFloat;
                    
                case 2:
                    return MTLVertexFormatFloat2;
                    
                case 3:
                    return MTLVertexFormatFloat3;
                    
                case 4:
                    return MTLVertexFormatFloat4;
                    
                default:
                    pabort();
                    return MTLVertexFormatFloat;
            }
            
        default:
            pabort();
            return MTLVertexFormatFloat;
    }
}

MTLPrimitiveType VROGeometrySubstrateMetal::parsePrimitiveType(VROGeometryPrimitiveType primitive) {
    switch (primitive) {
        case VROGeometryPrimitiveType::Triangle:
            return MTLPrimitiveTypeTriangle;
            
        case VROGeometryPrimitiveType::TriangleStrip:
            return MTLPrimitiveTypeTriangleStrip;
            
        case VROGeometryPrimitiveType::Line:
            return MTLPrimitiveTypeLine;
            
        case VROGeometryPrimitiveType::Point:
            return MTLPrimitiveTypePoint;
            
        default:
            break;
    }
}

int VROGeometrySubstrateMetal::parseAttributeIndex(VROGeometrySourceSemantic semantic) {
    switch (semantic) {
        case VROGeometrySourceSemantic::Vertex:
            return 0;
        case VROGeometrySourceSemantic::Normal:
            return 1;
        case VROGeometrySourceSemantic::Color:
            return 2;
        case VROGeometrySourceSemantic::Texcoord:
            return 3;
        case VROGeometrySourceSemantic::VertexCrease:
            return 4;
        case VROGeometrySourceSemantic::EdgeCrease:
            return 5;
        case VROGeometrySourceSemantic::BoneWeights:
            return 6;
        case VROGeometrySourceSemantic::BoneIndices:
            return 7;
        default:
            return 0;
    }
}

