//
//  VROGeometryUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROGeometryUtil.h"
#include "VROData.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROMath.h"
#include "VROLog.h"
#include "VRONode.h"
#include "VROGeometry.h"
#include <set>

std::vector<std::shared_ptr<VRONode>> VROGeometryUtilSplitNodeByGeometryElements(std::shared_ptr<VRONode> node) {
    std::vector<std::shared_ptr<VRONode>> nodes;
    std::shared_ptr<VROGeometry> geometry = node->getGeometry();
    
    for (int i = 0; i < geometry->getGeometryElements().size(); i++) {
        std::shared_ptr<VROGeometryElement> element = geometry->getGeometryElements()[i];
        std::vector<std::shared_ptr<VROGeometryElement>> elements = { element };
        
        std::shared_ptr<VROGeometrySource> vertexSource = geometry->getGeometrySourcesForSemantic(VROGeometrySourceSemantic::Vertex).front();
        
        VROVector3f center;
        std::shared_ptr<VROData> data = VROGeometryUtilExtractAndCenter(element, vertexSource, &center);
        
        std::vector<std::shared_ptr<VROGeometrySource>> sources;
        for (std::shared_ptr<VROGeometrySource> source : geometry->getGeometrySources()) {
            sources.push_back(std::make_shared<VROGeometrySource>(data, source));
        }
        
        std::shared_ptr<VROMaterial> material = geometry->getMaterials()[i];
        std::shared_ptr<VROGeometry> geometry = std::make_shared<VROGeometry>(sources, elements);
        geometry->getMaterials().push_back(material);
        
        std::shared_ptr<VRONode> splitNode = std::make_shared<VRONode>();
        splitNode->setGeometry(geometry);
        splitNode->setPosition(center);
        
        nodes.push_back(splitNode);
    }
    
    return nodes;
}

std::shared_ptr<VROData> VROGeometryUtilExtractAndCenter(std::shared_ptr<VROGeometryElement> element,
                                                         std::shared_ptr<VROGeometrySource> geometrySource,
                                                         VROVector3f *outCenter) {
    std::vector<VROVector3f> allVertices;
    geometrySource->processVertices([&allVertices](int index, VROVector3f vertex) {
        allVertices.push_back(vertex);
    });
    
    /*
     Find the center. Note that vertices may be referenced multiple times by
     an element, so we get the unique vertex indices first.
     */
    std::set<int> elementVertexIndices;
    element->processIndices([&allVertices, &elementVertexIndices](int index, int indexRead) {
        elementVertexIndices.insert(indexRead);
    });
    
    std::vector<VROVector3f> elementVertices;
    for (int elementVertexIndex : elementVertexIndices) {
        elementVertices.push_back(allVertices[elementVertexIndex]);
    }
    
    *outCenter = VROMathGetCenter(elementVertices);

    /*
     Subtract the center from each vertex in the element.
     */
    for (int elementVertexIndex : elementVertexIndices) {
        allVertices[elementVertexIndex] -= *outCenter;
    }
    
    std::shared_ptr<VROData> data = std::make_shared<VROData>(geometrySource->getData()->getData(),
                                                              geometrySource->getData()->getDataLength());
    
    std::shared_ptr<VROGeometrySource> source = std::make_shared<VROGeometrySource>(data, geometrySource);
    source->modifyVertices([&allVertices](int index, VROVector3f vertex) -> VROVector3f {
        return allVertices[index];
    });
    return source->getData();
}

int VROGeometryUtilGetIndicesCount(int primitiveCount, VROGeometryPrimitiveType primitiveType) {
    switch (primitiveType) {
        case VROGeometryPrimitiveType::Triangle:
            return primitiveCount * 3;
            
        case VROGeometryPrimitiveType::TriangleStrip:
            return primitiveCount + 2;
            
        case VROGeometryPrimitiveType::Line:
            return primitiveCount * 2;
            
        case VROGeometryPrimitiveType::Point:
            return primitiveCount;
            
        default:
            break;
    }
}

int VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic semantic) {
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