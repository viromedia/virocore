//
//  VROOBJLoader.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/13/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROOBJLoader.h"
#include "VROLog.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROTexture.h"
#include "VROMaterial.h"
#include "VROGeometry.h"
#include "VROPlatformUtil.h"
#include "VROGeometryUtil.h"
#include "VRONode.h"
#include "VROByteBuffer.h"
#include "tiny_obj_loader.h"
#include "VROShapeUtils.h"
#include "VROTaskQueue.h"
#include "VROModelIOUtil.h"

void VROOBJLoader::loadOBJFromResource(std::string resource, VROResourceType type,
                                       std::shared_ptr<VRONode> node,
                                       std::shared_ptr<VRODriver> driver,
                                       std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    VROModelIOUtil::retrieveResourceAsync(resource, type,
          [resource, type, node, driver, onFinish](std::string path, bool isTemp) {
              // onSuccess() (note: callbacks from retrieveResourceAsync occur on rendering thread)
              readOBJFileAsync(resource, type, node, path, isTemp, false, {}, driver, onFinish);
          },
          [node, onFinish]() {
              // onFailure()
              onFinish(node, false);
          });
}

void VROOBJLoader::loadOBJFromResources(std::string resource, VROResourceType type,
                                        std::shared_ptr<VRONode> node,
                                        std::map<std::string, std::string> resourceMap,
                                        std::shared_ptr<VRODriver> driver,
                                        std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    VROModelIOUtil::retrieveResourceAsync(resource, type,
          [resource, type, node, resourceMap, driver, onFinish](std::string path, bool isTemp) {
              // onSuccess() (note: callbacks from retrieveResourceAsync occur on rendering thread)
              readOBJFileAsync(resource, type, node, path, isTemp, true, resourceMap, driver, onFinish);
          },
          [node, onFinish]() {
              // onFailure()
              onFinish(node, false);
          });
}

void VROOBJLoader::readOBJFileAsync(std::string resource, VROResourceType type, std::shared_ptr<VRONode> node,
                                    std::string path, bool isTemp, bool loadingTexturesFromResourceMap,
                                    std::map<std::string, std::string> resourceMap,
                                    std::shared_ptr<VRODriver> driver,
                                    std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish) {
    VROPlatformDispatchAsyncBackground([resource, type, node, path, resourceMap, driver, onFinish, isTemp, loadingTexturesFromResourceMap] {
        pinfo("Loading OBJ from file %s", path.c_str());
        std::string base = resource.substr(0, resource.find_last_of('/'));

        tinyobj::attrib_t *attrib = new tinyobj::attrib_t();
        std::vector<tinyobj::shape_t> *shapes = new std::vector<tinyobj::shape_t>();
        std::vector<tinyobj::material_t> *materials = new std::vector<tinyobj::material_t>();
        
        /*
         If the ancillary resources (e.g. textures) required by the model are provided in a
         resource map, then generate the corresponding fileMap (this copies those resources
         into local files).
         */
        std::map<std::string, std::string> *fileMap = new std::map<std::string, std::string>();
        if (loadingTexturesFromResourceMap) {
            *fileMap = VROModelIOUtil::processResourceMap(resourceMap, type);
        }
        
        std::string err;
        tinyobj::LoadObj(attrib, shapes, materials, &err,
                         path.c_str(),
                         base.c_str(),
                         type == VROResourceType::URL, // base is URL?
                         loadingTexturesFromResourceMap ? fileMap : nullptr,
                         [err, node, attrib, shapes, materials, resource, type, loadingTexturesFromResourceMap,
                          fileMap, driver, onFinish](bool ret) {
                             if (!err.empty()) {
                                 pinfo("OBJ loading warning [%s]", err.c_str());
                             }
                             
                             if (ret) {
                                 std::string base = resource.substr(0, resource.find_last_of('/'));
                                 
                                 // Load the FBX from the protobuf on the rendering thread, accumulating additional
                                 // tasks (e.g. async texture download) in the task queue
                                 std::shared_ptr<VROTaskQueue> taskQueue = std::make_shared<VROTaskQueue>(VROTaskExecutionOrder::Serial);
                                 std::map<std::string, std::shared_ptr<VROTexture>> *textureCache = new std::map<std::string, std::shared_ptr<VROTexture>>();
                                 std::shared_ptr<VROGeometry> geo = processOBJ(*attrib, *shapes, *materials, base,
                                                                               loadingTexturesFromResourceMap ? VROResourceType::LocalFile : type,
                                                                               loadingTexturesFromResourceMap ? fileMap : nullptr,
                                                                               *textureCache, taskQueue);
                                 
                                 // Run all the async tasks. When they're complete, inject the finished FBX into the
                                 // node
                                 taskQueue->processTasksAsync([geo, node, attrib, shapes, materials, fileMap, textureCache, driver, onFinish] {
                                     injectOBJ(geo, node, driver, onFinish);
                                     delete (textureCache);
                                     delete (attrib);
                                     delete (shapes);
                                     delete (materials);
                                     delete (fileMap);
                                 });
                             }
                             else {
                                 pinfo("Failed to load OBJ data");
                                 onFinish(node, false);
                                 delete (attrib);
                                 delete (shapes);
                                 delete (materials);
                                 delete (fileMap);
                             }
                         });
        
        if (isTemp) {
            VROPlatformDeleteFile(path);
        }
    });
}

void VROOBJLoader::injectOBJ(std::shared_ptr<VROGeometry> geometry,
                             std::shared_ptr<VRONode> node,
                             std::shared_ptr<VRODriver> driver,
                             std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish) {
 
    if (geometry) {
        node->setGeometry(geometry);
        
        // recompute the node's umbrellaBoundingBox and set the atomic rendering properties before
        // we notify the user that their OBJ has finished loading
        node->recomputeUmbrellaBoundingBox();
        node->setAtomicRenderProperties();

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

std::shared_ptr<VROGeometry> VROOBJLoader::processOBJ(tinyobj::attrib_t &attrib,
                                                      std::vector<tinyobj::shape_t> &shapes,
                                                      std::vector<tinyobj::material_t> &materials,
                                                      std::string base,
                                                      VROResourceType type,
                                                      const std::map<std::string, std::string> *resourceMap,
                                                      std::map<std::string, std::shared_ptr<VROTexture>> &textureCache,
                                                      std::shared_ptr<VROTaskQueue> taskQueue) {
    pinfo("OBJ # of vertices  = %d", (int)(attrib.vertices.size()) / 3);
    pinfo("OBJ # of normals   = %d", (int)(attrib.normals.size()) / 3);
    pinfo("OBJ # of texcoords = %d", (int)(attrib.texcoords.size()) / 2);
    pinfo("OBJ # of materials = %d", (int)materials.size());
    pinfo("OBJ # of shapes    = %d", (int)shapes.size());
    
    /*
     Load materials, if provided, creating a VROMaterial for each OBJ material.
     */
    std::vector<std::shared_ptr<VROMaterial>> materialsIndexed;
    
    for (tinyobj::material_t &m : materials) {
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        material->setName(m.name);
      
        if (m.has_diffuse_color) {
            material->getDiffuse().setColor({ m.diffuse, 3 });
        }
        material->setShininess(m.shininess);
        material->setTransparency(m.dissolve);
        
        if (m.has_illum) {
            if (m.illum == 0) {
                material->setLightingModel(VROLightingModel::Constant);
            }
            else if (m.illum == 1) {
                material->setLightingModel(VROLightingModel::Lambert);
            }
            else if (m.illum >= 2) {
                material->setLightingModel(VROLightingModel::Blinn);
            }
            // Future, support additional illumination models: http://paulbourke.net/dataformats/mtl/
        }
        else {
            // If no illumination model given, default to Blinn
            material->setLightingModel(VROLightingModel::Blinn);
        }
        
        std::string diffuseTexname = m.diffuse_texname;
        if (diffuseTexname.length() > 0) {
            taskQueue->addTask([material, diffuseTexname, base, type, resourceMap, &textureCache, taskQueue] {
                VROModelIOUtil::loadTextureAsync(diffuseTexname, base, type, true, resourceMap, textureCache,
                     [material, taskQueue, diffuseTexname](std::shared_ptr<VROTexture> texture) {
                         if (texture) {
                             material->getDiffuse().setTexture(texture);
                         }
                         else {
                             pinfo("Failed to load diffuse texture [%s] for OBJ", diffuseTexname.c_str());
                         }
                         taskQueue->onTaskComplete();
                     });
            });
        }
        
        std::string specularTexname = m.specular_texname;
        if (specularTexname.length() > 0) {
            taskQueue->addTask([material, specularTexname, base, type, resourceMap, &textureCache, taskQueue] {
                VROModelIOUtil::loadTextureAsync(specularTexname, base, type, false, resourceMap, textureCache,
                     [material, taskQueue, specularTexname](std::shared_ptr<VROTexture> texture) {
                         if (texture) {
                             material->getSpecular().setTexture(texture);
                         }
                         else {
                             pinfo("Failed to load specular texture [%s] for OBJ", specularTexname.c_str());
                         }
                         taskQueue->onTaskComplete();
                     });
            });
        }
        
        std::string normalTexname = m.bump_texname;
        if (normalTexname.length() > 0) {
            taskQueue->addTask([material, normalTexname, base, type, resourceMap, &textureCache, taskQueue] {
                VROModelIOUtil::loadTextureAsync(normalTexname, base, type, false, resourceMap, textureCache,
                     [material, taskQueue, normalTexname](std::shared_ptr<VROTexture> texture) {
                         if (texture) {
                             material->getNormal().setTexture(texture);
                         }
                         else {
                             pinfo("Failed to load normal map texture [%s] for OBJ", normalTexname.c_str());
                         }
                         taskQueue->onTaskComplete();
                     });
            });
        }

        // PBR values
        material->getRoughness().setColor({ m.roughness, m.roughness, m.roughness, 1.0 });
        std::string roughnessTexname = m.roughness_texname;
        if (roughnessTexname.length() > 0) {
            taskQueue->addTask([material, roughnessTexname, base, type, resourceMap, &textureCache, taskQueue] {
                VROModelIOUtil::loadTextureAsync(roughnessTexname, base, type, false, resourceMap, textureCache,
                     [material, taskQueue, roughnessTexname](std::shared_ptr<VROTexture> texture) {
                         if (texture) {
                             material->getRoughness().setTexture(texture);
                         }
                         else {
                             pinfo("Failed to load roughness map [%s] for OBJ", roughnessTexname.c_str());
                         }
                         taskQueue->onTaskComplete();
                     });
            });
        }

        material->getMetalness().setColor({ m.metallic, m.metallic, m.metallic, 1.0 });
        std::string metalnessTexname = m.metallic_texname;
        if (metalnessTexname.length() > 0) {
            taskQueue->addTask([material, metalnessTexname, base, type, resourceMap, &textureCache, taskQueue] {
                VROModelIOUtil::loadTextureAsync(metalnessTexname, base, type, false, resourceMap, textureCache,
                     [material, taskQueue, metalnessTexname](std::shared_ptr<VROTexture> texture) {
                         if (texture) {
                             material->getMetalness().setTexture(texture);
                         }
                         else {
                             pinfo("Failed to load metalness map [%s] for OBJ", metalnessTexname.c_str());
                         }
                         taskQueue->onTaskComplete();
                     });
            });
        }

        materialsIndexed.push_back(material);
    }
    
    /*
     Build a single interleaved vertex array. All geometry elements will point to this
     array.
     */
    std::vector<float> &vertices = attrib.vertices;
    std::vector<float> &texcoords = attrib.texcoords;
    std::vector<float> &normals = attrib.normals;
    
    /*
     Create a default material. This material will be used for the elements created
     from shapes that have no material specified. If there is no MTL file, then
     this material will be set for all elements.
     */
    std::shared_ptr<VROMaterial> defaultMaterial = std::make_shared<VROMaterial>();
    defaultMaterial->setName("OBJ Default");
    
    std::vector<std::shared_ptr<VROMaterial>> elementMaterials;
    
    /*
     Count the number of vertices in the OBJ.
     */
    int numVertices = 0;
    for (tinyobj::shape_t &shape : shapes) {
        tinyobj::mesh_t &mesh = shape.mesh;
        
        for (int f = 0; f < mesh.num_face_vertices.size(); f++) {
            numVertices += mesh.num_face_vertices[f];
        }
    }
    
    int stride = sizeof(VROShapeVertexLayout);
    int varSizeBytes = numVertices * stride;
    
    // Will be moved to VROData so does not need to be explicitly freed!
    VROShapeVertexLayout *var = (VROShapeVertexLayout *) malloc(varSizeBytes);
    VROVector3f *tangents = VROShapeUtilStartTangents(var, numVertices);

    std::vector<std::shared_ptr<VROGeometryElement>> elements;

    int currentVertex = 0;
    int currentIndex = 0;
    for (tinyobj::shape_t &shape : shapes) {
        tinyobj::mesh_t &mesh = shape.mesh;
        
        /*
         Write all the indices of this mesh into the VAR layout.
         */
        for (int i = 0; i < mesh.indices.size(); i++) {
            tinyobj::index_t &index = mesh.indices[i];
            
            VROShapeVertexLayout &v = var[currentVertex];
            
            v.x = vertices[index.vertex_index * 3 + 0];
            v.y = vertices[index.vertex_index * 3 + 1];
            v.z = vertices[index.vertex_index * 3 + 2];
            
            if (index.texcoord_index >= 0) {
                v.u = texcoords[index.texcoord_index * 2 + 0];
                v.v = 1 - texcoords[index.texcoord_index * 2 + 1];
            }
            else {
                v.u = 0;
                v.v = 0;
            }
            
            if (index.normal_index >= 0) {
                v.nx = normals[index.normal_index * 3 + 0];
                v.ny = normals[index.normal_index * 3 + 1];
                v.nz = normals[index.normal_index * 3 + 2];
            }
            else {
                v.nx = 0;
                v.ny = 0;
                v.nz = 0;
            }
            
            ++currentVertex;
        }

        /*
         Create one element for each material used by this shape.
         Maps from material index to indices using said material.
         */
        std::map<int, std::vector<int>> indicesByMaterial;
        
        for (int f = 0; f < mesh.num_face_vertices.size(); f++) {
            int numFaceVertices = mesh.num_face_vertices[f];
            if (numFaceVertices != 3) {
                pinfo("Non-triangular OBJ files not supported!");
                return {};
            }
            
            std::vector<int> &indices = indicesByMaterial[mesh.material_ids[f]];
            for (int c = 0; c < numFaceVertices; c++) {
                indices.push_back(currentIndex++);
            }
        }
        
        for (auto &kv : indicesByMaterial) {
            std::vector<int> &indices = kv.second;
            
            VROGeometryPrimitiveType primitive = VROGeometryPrimitiveType::Triangle;
            int indexCount = (int) indices.size();
            int bytesPerIndex = sizeof(int);
            int primitiveCount = VROGeometryUtilGetPrimitiveCount(indexCount, primitive);
            std::shared_ptr<VROData> data = std::make_shared<VROData>(indices.data(), indexCount * bytesPerIndex);
            
            std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(data,
                                                                                               primitive,
                                                                                               primitiveCount,
                                                                                               bytesPerIndex);
            elements.push_back(element);
            
            /*
             Material index -1 corresponds to no MTL file, or no material set for the
             group of faces. In this case we set the default material.
             */
            int materialIndex = kv.first;
            if (materialIndex >= 0) {
                elementMaterials.push_back(materialsIndexed[kv.first]);
            }
            else {
                elementMaterials.push_back(defaultMaterial);
            }
            
            VROShapeUtilComputeTangentsForIndices(var, numVertices, indices.data(), indexCount, tangents);
        }
    }
    VROShapeUtilEndTangents(var, numVertices, tangents);    
    
    /*
     Now turn the interleaved array into three sources.
     */
    passert (numVertices == currentVertex);
    passert (numVertices == currentIndex);
    
    std::shared_ptr<VROData> data = std::make_shared<VROData>((void *) var, sizeof(VROShapeVertexLayout) * numVertices, VRODataOwnership::Move);
    std::vector<std::shared_ptr<VROGeometrySource>> sources = VROShapeUtilBuildGeometrySources(data, numVertices);
    
    std::shared_ptr<VROGeometry> geometry = std::make_shared<VROGeometry>(sources, elements);
    geometry->setMaterials(elementMaterials);
    
    VROBoundingBox bounds = geometry->getBoundingBox();
    pinfo("OBJ bounding box    =  x(%f %f) y(%f %f) z(%f %f)",
          bounds.getMinX(), bounds.getMaxX(),
          bounds.getMinY(), bounds.getMaxY(),
          bounds.getMinZ(), bounds.getMaxZ());
    
    return geometry;
}
