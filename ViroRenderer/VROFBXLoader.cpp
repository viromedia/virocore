//
//  VROFBXLoader.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROFBXLoader.h"
#include "VRONode.h"
#include "VROPlatformUtil.h"
#include "VROGeometry.h"
#include "VROData.h"
#include "VROModelIOUtil.h"
#include "Nodes.pb.h"

VROGeometrySourceSemantic convert(viro::Node_Geometry_Source_Semantic semantic) {
  switch (semantic) {
    case viro::Node_Geometry_Source_Semantic_Vertex:
      return VROGeometrySourceSemantic::Vertex;
    case viro::Node_Geometry_Source_Semantic_Normal:
      return VROGeometrySourceSemantic::Normal;
    case viro::Node_Geometry_Source_Semantic_Color:
      return VROGeometrySourceSemantic::Color;
    case viro::Node_Geometry_Source_Semantic_Texcoord:
      return VROGeometrySourceSemantic::Texcoord;
    case viro::Node_Geometry_Source_Semantic_Tangent:
      return VROGeometrySourceSemantic::Tangent;
    case viro::Node_Geometry_Source_Semantic_VertexCrease:
      return VROGeometrySourceSemantic::VertexCrease;
    case viro::Node_Geometry_Source_Semantic_EdgeCrease:
      return VROGeometrySourceSemantic::EdgeCrease;
    case viro::Node_Geometry_Source_Semantic_BoneWeights:
      return VROGeometrySourceSemantic::BoneWeights;
    case viro::Node_Geometry_Source_Semantic_BoneIndices:
      return VROGeometrySourceSemantic::BoneIndices;
    default:
      pabort();
  }
}

VROGeometryPrimitiveType convert(viro::Node_Geometry_Element_Primitive primitive) {
  switch (primitive) {
    case viro::Node_Geometry_Element_Primitive_Triangle:
      return VROGeometryPrimitiveType::Triangle;
    case viro::Node_Geometry_Element_Primitive_TriangleStrip:
      return VROGeometryPrimitiveType::TriangleStrip;
    case viro::Node_Geometry_Element_Primitive_Line:
      return VROGeometryPrimitiveType::Line;
    case viro::Node_Geometry_Element_Primitive_Point:
      return VROGeometryPrimitiveType::Point;
    default:
      pabort();
  }
}

VROLightingModel convert(viro::Node_Geometry_Material_LightingModel lightingModel) {
  switch (lightingModel) {
    case viro::Node_Geometry_Material_LightingModel_Constant:
      return VROLightingModel::Constant;
    case viro::Node_Geometry_Material_LightingModel_Lambert:
      return VROLightingModel::Lambert;
    case viro::Node_Geometry_Material_LightingModel_Blinn:
      return VROLightingModel::Blinn;
    case viro::Node_Geometry_Material_LightingModel_Phong:
      return VROLightingModel::Phong;
    default:
      pabort();
  }
}

std::shared_ptr<VRONode> VROFBXLoader::loadFBXFromURL(std::string url, std::string baseURL,
                                                      bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([url, baseURL, node, onFinish] {
            bool isTemp = false;
            bool success = false;
            std::string file = VROPlatformDownloadURLToFile(url, &isTemp, &success);
            
            std::shared_ptr<VRONode> fbxNode;
            if (success) {
                fbxNode = loadFBX(file, baseURL, true, nullptr);
            }
            if (isTemp) {
                VROPlatformDeleteFile(file);
            }
            
            VROPlatformDispatchAsyncRenderer([node, fbxNode, onFinish] {
                injectFBX(fbxNode, node, onFinish);
            });
        });
    }
    else {
        bool isTemp = false;
        bool success = false;
        std::string file = VROPlatformDownloadURLToFile(url, &isTemp, &success);
        
        std::shared_ptr<VRONode> fbxNode;
        if (success) {
            fbxNode = loadFBX(file, baseURL, true, nullptr);
        }
        if (isTemp) {
            VROPlatformDeleteFile(file);
        }
        
        injectFBX(fbxNode, node, onFinish);
    }
    
    return node;
}

std::shared_ptr<VRONode> VROFBXLoader::loadFBXFromFile(std::string file, std::string baseDir,
                                                       bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([file, baseDir, node, onFinish] {
            std::shared_ptr<VRONode> fbxNode = loadFBX(file, baseDir, false, nullptr);
            VROPlatformDispatchAsyncRenderer([node, fbxNode, onFinish] {
                injectFBX(fbxNode, node, onFinish);
            });
        });
    }
    else {
        std::shared_ptr<VRONode> fbxNode = loadFBX(file, baseDir, false, nullptr);
        injectFBX(fbxNode, node, onFinish);
    }
    
    return node;
}

std::shared_ptr<VRONode> VROFBXLoader::loadFBXFromFileWithResources(std::string file, std::map<std::string, std::string> resourceMap,
                                                                    bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([file, resourceMap, node, onFinish] {
            std::shared_ptr<VRONode> fbxNode = loadFBX(file, "", false, &resourceMap);
            VROPlatformDispatchAsyncRenderer([node, fbxNode, onFinish] {
                injectFBX(fbxNode, node, onFinish);
            });
        });
    }
    else {
        std::shared_ptr<VRONode> fbxNode = loadFBX(file, "", false, &resourceMap);
        injectFBX(fbxNode, node, onFinish);
    }
    
    return node;
}

void VROFBXLoader::injectFBX(std::shared_ptr<VRONode> fbxNode, std::shared_ptr<VRONode> node,
                             std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish) {
    
    if (fbxNode) {
        node->setPosition(fbxNode->getPosition());
        node->setScale(fbxNode->getScale());
        node->setRotation(fbxNode->getRotation());
        node->setRenderingOrder(fbxNode->getRenderingOrder());
        node->setOpacity(fbxNode->getOpacity());
        
        node->setGeometry(fbxNode->getGeometry());
        for (std::shared_ptr<VRONode> child : fbxNode->getSubnodes()) {
            node->addChildNode(child);
        }
        
        if (onFinish) {
            onFinish(node, true);
        }
    }
    else {
        if (onFinish) {
            onFinish(node, false);
        }
    }
}

std::shared_ptr<VRONode> VROFBXLoader::loadFBX(std::string file, std::string base, bool isBaseURL,
                                               const std::map<std::string, std::string> *resourceMap) {
    
    std::map<std::string, std::shared_ptr<VROTexture>> textureCache;

    pinfo("Loading FBX from file %s", file.c_str());
    std::string data_pb = VROPlatformLoadFileAsString(file);
    
    viro::Node node_pb;
    if (!node_pb.ParseFromString(data_pb)) {
        pinfo("Failed to parse FBX protobuf");
        return {};
    }
    
    pinfo("Read FBX protobuf");
    
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    node->setThreadRestrictionEnabled(false);
    node->setPosition({ node_pb.position(0), node_pb.position(1), node_pb.position(2) });
    node->setScale({ node_pb.scale(0), node_pb.scale(1), node_pb.scale(2) });
    node->setRotation({ node_pb.rotation(0), node_pb.rotation(1), node_pb.rotation(2) });
    node->setRenderingOrder(node_pb.rendering_order());
    node->setOpacity(node_pb.opacity());
    
    const viro::Node::Geometry &geo_pb = node_pb.geometry();
    std::shared_ptr<VROData> varData = std::make_shared<VROData>(geo_pb.data().c_str(), geo_pb.data().length());
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    for (int i = 0; i < geo_pb.source_size(); i++) {
        const viro::Node::Geometry::Source &source_pb = geo_pb.source(i);
        std::shared_ptr<VROGeometrySource> source = std::make_shared<VROGeometrySource>(varData,
                                                                                        convert(source_pb.semantic()),
                                                                                        source_pb.vertex_count(),
                                                                                        source_pb.float_components(),
                                                                                        source_pb.components_per_vertex(),
                                                                                        source_pb.bytes_per_component(),
                                                                                        source_pb.data_offset(),
                                                                                        source_pb.data_stride());
        sources.push_back(source);
    }
    
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    for (int i = 0; i < geo_pb.element_size(); i++) {
        const viro::Node::Geometry::Element &element_pb = geo_pb.element(i);
        
        std::shared_ptr<VROData> data = std::make_shared<VROData>(element_pb.data().c_str(), element_pb.data().length());
        std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(data,
                                                                                           convert(element_pb.primitive()),
                                                                                           element_pb.primitive_count(),
                                                                                           element_pb.bytes_per_index());
        elements.push_back(element);
    }
    
    std::shared_ptr<VROGeometry> geo = std::make_shared<VROGeometry>(sources, elements);
    geo->setName(geo_pb.name());
    
    std::vector<std::shared_ptr<VROMaterial>> materials;
    for (int i = 0; i < geo_pb.material_size(); i++) {
        const viro::Node::Geometry::Material &material_pb = geo_pb.material(i);
        
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        material->setName(material_pb.name());
        material->setShininess(material_pb.shininess());
        material->setFresnelExponent(material_pb.fresnel_exponent());
        material->setTransparency(material_pb.transparency());
        material->setLightingModel(convert(material_pb.lighting_model()));
        material->setReadsFromDepthBuffer(true);
        material->setWritesToDepthBuffer(true);
        
        if (material_pb.has_diffuse()) {
            const viro::Node::Geometry::Material::Visual &diffuse_pb = material_pb.diffuse();
            VROMaterialVisual &diffuse = material->getDiffuse();

            diffuse.setColor({ diffuse_pb.color(0), diffuse_pb.color(1), diffuse_pb.color(2), 1.0 });
            diffuse.setIntensity(diffuse_pb.intensity());
            
            if (!diffuse_pb.texture().empty()) {
                std::shared_ptr<VROTexture> texture = VROModelIOUtil::loadTexture(diffuse_pb.texture(), base, isBaseURL, resourceMap, textureCache);
                if (texture) {
                    diffuse.setTexture(texture);
                }
                else {
                    pinfo("FBX failed to load diffuse texture [%s]", diffuse_pb.texture().c_str());
                }
            }
        }
        if (material_pb.has_specular()) {
            const viro::Node::Geometry::Material::Visual &specular_pb = material_pb.specular();
            VROMaterialVisual &specular = material->getSpecular();

            specular.setIntensity(specular_pb.intensity());
            
            if (!specular_pb.texture().empty()) {
                std::shared_ptr<VROTexture> texture = VROModelIOUtil::loadTexture(specular_pb.texture(), base, isBaseURL, resourceMap, textureCache);
                if (texture) {
                    specular.setTexture(texture);
                }
                else {
                    pinfo("FBX failed to load specular texture [%s]", specular_pb.texture().c_str());
                }
            }
        }
        if (material_pb.has_normal()) {
            const viro::Node::Geometry::Material::Visual &normal_pb = material_pb.normal();
            VROMaterialVisual &normal = material->getNormal();

            normal.setIntensity(normal_pb.intensity());
            
            if (!normal_pb.texture().empty()) {
                std::shared_ptr<VROTexture> texture = VROModelIOUtil::loadTexture(normal_pb.texture(), base, isBaseURL, resourceMap, textureCache);
                if (texture) {
                    normal.setTexture(texture);
                }
                else {
                    pinfo("FBX failed to load normal texture [%s]", normal_pb.texture().c_str());
                }
            }
        }
        
        materials.push_back(material);
    }
    geo->setMaterials(materials);
    
    VROBoundingBox bounds = geo->getBoundingBox();
    pinfo("FBX bounding box    =  x(%f %f) y(%f %f) z(%f %f)",
          bounds.getMinX(), bounds.getMaxX(),
          bounds.getMinY(), bounds.getMaxY(),
          bounds.getMinZ(), bounds.getMaxZ());
    
    node->setGeometry(geo);
    return node;
}
