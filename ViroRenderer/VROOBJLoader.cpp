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

std::shared_ptr<VRONode> VROOBJLoader::loadOBJFromURL(std::string url, std::string baseURL,
                                                      bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([url, baseURL, node, onFinish] {
            bool isTemp = false;
            bool success = false;
            std::string file = VROPlatformDownloadURLToFile(url, &isTemp, &success);
            
            std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseURL, true);
            if (isTemp) {
                VROPlatformDeleteFile(file);
            }

            VROPlatformDispatchAsyncRenderer([node, geometry, onFinish] {
                injectOBJ(geometry, node, onFinish);
            });
        });
    }
    else {
        bool isTemp = false;
        bool success = false;
        std::string file = VROPlatformDownloadURLToFile(url, &isTemp, &success);
        
        std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseURL, true);
        if (isTemp) {
            VROPlatformDeleteFile(file);
        }
        
        injectOBJ(geometry, node, onFinish);
    }
    
    return node;
}

std::shared_ptr<VRONode> VROOBJLoader::loadOBJFromFile(std::string file, std::string baseDir,
                                                       bool async, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {

    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    
    if (async) {
        VROPlatformDispatchAsyncBackground([file, baseDir, node, onFinish] {
            std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseDir, false);
            VROPlatformDispatchAsyncRenderer([node, geometry, onFinish] {
                injectOBJ(geometry, node, onFinish);
            });
        });
    }
    else {
        std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseDir, false);
        injectOBJ(geometry, node, onFinish);
    }
    
    return node;
}

void VROOBJLoader::injectOBJ(std::shared_ptr<VROGeometry> geometry,
                             std::shared_ptr<VRONode> node,
                             std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish) {
 
    if (geometry) {
        node->setGeometry(geometry);
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

std::shared_ptr<VROGeometry> VROOBJLoader::loadOBJ(std::string file, std::string base,
                                                   bool isBaseURL) {
    pinfo("Loading OBJ from file %s", file.c_str());
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str(),
                                base.c_str(), isBaseURL);
    if (!err.empty()) {
        pinfo("OBJ loading warning [%s]", err.c_str());
    }
    
    if (!ret) {
        pabort("Failed to load OBJ");
        return {};
    }
    
    pinfo("OBJ # of vertices  = %d", (int)(attrib.vertices.size()) / 3);
    pinfo("OBJ # of normals   = %d", (int)(attrib.normals.size()) / 3);
    pinfo("OBJ # of texcoords = %d", (int)(attrib.texcoords.size()) / 2);
    pinfo("OBJ # of materials = %d", (int)materials.size());
    pinfo("OBJ # of shapes    = %d", (int)shapes.size());
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    
    /*
     Load materials, if provided, creating a VROMaterial for each OBJ material.
     */
    std::map<std::string, std::shared_ptr<VROTexture>> textures;
    std::vector<std::shared_ptr<VROMaterial>> materialsIndexed;
    
    for (tinyobj::material_t &m : materials) {
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        material->setName(m.name);
      
        if (m.has_diffuse_color) {
            material->getDiffuse().setColor({ m.diffuse, 3 });
        }
        material->setShininess(m.shininess);
        material->setTransparency(m.dissolve);
        
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
        
        if (m.diffuse_texname.length() > 0) {
            std::shared_ptr<VROTexture> texture = loadTexture(m.diffuse_texname, base, isBaseURL, textures);
            if (texture) {
                material->getDiffuse().setTexture(texture);
            }
            else {
                pinfo("Failed to load texture [%s] for OBJ", m.diffuse_texname.c_str());
            }
        }
        
        if (m.specular_texname.length() > 0) {
            std::shared_ptr<VROTexture> texture = loadTexture(m.specular_texname, base, isBaseURL, textures);
            if (texture) {
                material->getSpecular().setTexture(texture);
            }
            else {
                pinfo("Failed to load texture [%s] for OBJ", m.specular_texname.c_str());
            }
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
    
    int stride = 8 * sizeof(float);
    VROByteBuffer interleaved;

    int currentIndex = 0;
    for (tinyobj::shape_t &shape : shapes) {
        tinyobj::mesh_t &mesh = shape.mesh;
        
        /*
         Write all the indices of this mesh into the interleaved
         array.
         */
        interleaved.grow(stride * mesh.indices.size());
        
        for (int i = 0; i < mesh.indices.size(); i++) {
            tinyobj::index_t &index = mesh.indices[i];
            
            interleaved.writeFloat(vertices[index.vertex_index * 3 + 0]);
            interleaved.writeFloat(vertices[index.vertex_index * 3 + 1]);
            interleaved.writeFloat(vertices[index.vertex_index * 3 + 2]);
            
            if (index.texcoord_index >= 0) {
                interleaved.writeFloat(texcoords[index.texcoord_index * 2 + 0]);
                interleaved.writeFloat(1 - texcoords[index.texcoord_index * 2 + 1]);
            }
            else {
                interleaved.writeFloat(0);
                interleaved.writeFloat(0);
            }
            
            if (index.normal_index >= 0) {
                interleaved.writeFloat(normals[index.normal_index * 3 + 0]);
                interleaved.writeFloat(normals[index.normal_index * 3 + 1]);
                interleaved.writeFloat(normals[index.normal_index * 3 + 2]);
            }
            else {
                interleaved.writeFloat(0);
                interleaved.writeFloat(0);
                interleaved.writeFloat(0);
            }
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
        }
    }
    
    /*
     Now turn the interleaved array into three sources.
     */
    int numVertices = currentIndex;
    passert (numVertices == interleaved.getPosition() / stride);
    
    std::shared_ptr<VROData> data = std::make_shared<VROData>(interleaved.getData(), interleaved.getPosition());
    
    std::shared_ptr<VROGeometrySource> vertexSource = std::make_shared<VROGeometrySource>(data,
                                                                                          VROGeometrySourceSemantic::Vertex,
                                                                                          numVertices,
                                                                                          true,
                                                                                          3,
                                                                                          sizeof(float),
                                                                                          0,
                                                                                          stride);
    sources.push_back(vertexSource);
    
    std::shared_ptr<VROGeometrySource> texcoordSource = std::make_shared<VROGeometrySource>(data,
                                                                                            VROGeometrySourceSemantic::Texcoord,
                                                                                            numVertices,
                                                                                            true,
                                                                                            2,
                                                                                            sizeof(float),
                                                                                            sizeof(float) * 3,
                                                                                            stride);
    sources.push_back(texcoordSource);
    
    std::shared_ptr<VROGeometrySource> normalsSource = std::make_shared<VROGeometrySource>(data,
                                                                                           VROGeometrySourceSemantic::Normal,
                                                                                           numVertices,
                                                                                           true,
                                                                                           3,
                                                                                           sizeof(float),
                                                                                           sizeof(float) * 5,
                                                                                           stride);
    sources.push_back(normalsSource);

    std::shared_ptr<VROGeometry> geometry = std::make_shared<VROGeometry>(sources, elements);
    for (std::shared_ptr<VROMaterial> &material : elementMaterials) {
        geometry->getMaterials().push_back(material);
    }
    
    return geometry;
}

std::shared_ptr<VROTexture> VROOBJLoader::loadTexture(std::string &name, std::string &base, bool isBaseURL,
                                                      std::map<std::string, std::shared_ptr<VROTexture>> &cache) {
    std::shared_ptr<VROTexture> texture;
    
    auto it = cache.find(name);
    if (it == cache.end()) {
        bool isTempTextureFile = false;
        std::string textureFile = base + "/" + name;
        if (isBaseURL) {
            bool success = false;
            textureFile = VROPlatformDownloadURLToFile(textureFile, &isTempTextureFile, &success);
        }
        
        // Abort (return empty texture) if the file wasn't found
        if (textureFile.length() == 0) {
            return texture;
        }
        
        std::shared_ptr<VROImage> image = VROPlatformLoadImageFromFile(textureFile,
                                                                       VROTextureInternalFormat::RGBA8);
        if (isTempTextureFile) {
            VROPlatformDeleteFile(textureFile);
        }
        
        texture = std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8, VROMipmapMode::Runtime, image);
        cache.insert(std::make_pair(name, texture));
    }
    else {
        texture = it->second;
    }
    
    return texture;
}
