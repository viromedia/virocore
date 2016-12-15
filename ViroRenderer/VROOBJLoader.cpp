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
            std::string file = VROPlatformDownloadURLToFile(url, &isTemp);
            
            // TODO Viro-669 baseURL isn't used yet but needs to be converted to baseDir somehow
            std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseURL);
            if (isTemp) {
                VROPlatformDeleteFile(file);
            }
            
            VROPlatformDispatchAsyncMain([node, geometry, onFinish] {
                injectOBJ(geometry, node, onFinish);
            });
        });
    }
    else {
        bool isTemp = false;
        std::string file = VROPlatformDownloadURLToFile(url, &isTemp);
        
        // TODO Viro-669 baseURL isn't used yet but needs to be converted to baseDir somehow
        std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseURL);
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
            std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseDir);
            
            VROPlatformDispatchAsyncMain([node, geometry, onFinish] {
                injectOBJ(geometry, node, onFinish);
            });
        });
    }
    else {
        std::shared_ptr<VROGeometry> geometry = loadOBJ(file, baseDir);
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

std::shared_ptr<VROGeometry> VROOBJLoader::loadOBJ(std::string file, std::string baseDir) {
    pinfo("Loading OBJ from file %s", file.c_str());
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str(), nullptr);
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
    std::map<std::string, std::shared_ptr<VROTexture>> textures;
    
    /*
     Load materials, which come from the MTL file. Not currently supported.
     Code left commented out because this will serve as the stub for future
     work, tracked by VIRO-669.
     */
    /*
     for (tinyobj::material_t &mp : materials) {
        if (mp.diffuse_texname.length() > 0) {
        // Only load the texture if it is not already loaded
            if (textures.find(mp.diffuse_texname) == textures.end()) {
     
                std::string texture_filename = mp.diffuse_texname;
                std::string texURL = baseURL + "/" + texture_filename;
     
                bool texFileTemp = false;
                std::string texFile = VROPlatformDownloadURLToFile(texURL, &texFileTemp);
     
                std::shared_ptr<VROImage> image = VROPlatformLoadImageFromFile(texFile);
                if (texFileTemp) {
                    VROPlatformDeleteFile(texFile);
                }
     
                std::shared_ptr<VROTexture> texture = std::make_shared<VROTexture>(image);
                textures.insert(std::make_pair(mp.diffuse_texname, texture));
            }
        }
     }
     */
    
    /*
     Build a single interleaved vertex array. All geometry elements will point to this
     array.
     */
    std::vector<float> &vertices = attrib.vertices;
    std::vector<float> &texcoords = attrib.texcoords;
    std::vector<float> &normals = attrib.normals;
    
    int stride = 8 * sizeof(float);
    VROByteBuffer interleaved;
    
    int currentIndex = 0;
    for (tinyobj::shape_t &shape : shapes) {
        tinyobj::mesh_t &mesh = shape.mesh;
        
        std::vector<int> indices;
        interleaved.grow(stride * mesh.indices.size());
        
        for (int i = 0; i < mesh.indices.size(); i++) {
            tinyobj::index_t &index = mesh.indices[i];
            
            interleaved.writeFloat(vertices[index.vertex_index * 3 + 0]);
            interleaved.writeFloat(vertices[index.vertex_index * 3 + 1]);
            interleaved.writeFloat(vertices[index.vertex_index * 3 + 2]);
            interleaved.writeFloat(texcoords[index.texcoord_index * 2 + 0]);
            interleaved.writeFloat(texcoords[index.texcoord_index * 2 + 1]);
            interleaved.writeFloat(normals[index.normal_index * 3 + 0]);
            interleaved.writeFloat(normals[index.normal_index * 3 + 1]);
            interleaved.writeFloat(normals[index.normal_index * 3 + 2]);
            
            indices.push_back(currentIndex);
            ++currentIndex;
        }
        
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
    
    return std::make_shared<VROGeometry>(sources, elements);
}



