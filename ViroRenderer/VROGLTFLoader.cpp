//
//  VROGLTFLoader.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#define TINYGLTF_IMPLEMENTATION 1
#define TINYGLTF_NO_STB_IMAGE_WRITE 1
#define TINYGLTF_NO_STB_IMAGE 1

#include "VROGLTFLoader.h"
#include "tiny_gltf.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include "VRONode.h"
#include "VROByteBuffer.h"
#include "VROGeometry.h"
#include "VROMaterial.h"
#include "VROBox.h"
#include "VROGeometryUtil.h"
#include "VROSurface.h"
#include "VROTestUtil.h"
#include "VROCompress.h"
#include "VROModelIOUtil.h"

std::map<std::string, std::shared_ptr<VROData>> VROGLTFLoader::_dataCache;
std::map<std::string, std::shared_ptr<VROTexture>> VROGLTFLoader::_textureCache;

VROGeometrySourceSemantic VROGLTFLoader::getGeometryAttribute(std::string name) {
    if (VROStringUtil::strcmpinsensitive(name, "POSITION")) {
        return VROGeometrySourceSemantic::Vertex;
    } else if (VROStringUtil::strcmpinsensitive(name, "NORMAL")) {
        return VROGeometrySourceSemantic::Normal;
    } else if (VROStringUtil::strcmpinsensitive(name, "TANGENT")) {
        return VROGeometrySourceSemantic::Tangent;
    } else if (VROStringUtil::strcmpinsensitive(name, "TEXCOORD_0")) {
        return VROGeometrySourceSemantic::Texcoord;
    } else if (VROStringUtil::strcmpinsensitive(name, "COLOR_0")) {
        return VROGeometrySourceSemantic::Color;
    } else if (VROStringUtil::strcmpinsensitive(name, "JOINTS_0")) {
        //TODO VIRO-3627: GLTF Animations
        pwarn("GTLF animations are not yet supported! Unable to parse Joints attributes.");
    } else if (VROStringUtil::strcmpinsensitive(name, "WEIGHTS_0")) {
        //TODO VIRO-3627: GLTF Animations
        pwarn("GTLF animations are not yet supported! Unable to parse Weighted attributes.");
    } else {
        pwarn("Atempted to parse an unknown geometry attribute: %s", name.c_str());
    }
    return VROGeometrySourceSemantic::Invalid;
}

bool VROGLTFLoader::getComponentType(const tinygltf::Accessor &gAccessor, GLTFTypeComponent &typeComponent) {
    switch (gAccessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            typeComponent = GLTFTypeComponent::Byte;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            typeComponent = GLTFTypeComponent::UnsignedByte;
            break;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            typeComponent = GLTFTypeComponent::Short;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            typeComponent = GLTFTypeComponent::UnsignedShort;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            typeComponent = GLTFTypeComponent::UnsignedInt;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            typeComponent = GLTFTypeComponent::Float;
            break;
        default:
            perr("Unsupported Component type was provided to the GLTF Loader!");
            return false;
    }
    return true;
}

bool VROGLTFLoader::getComponent(const tinygltf::Accessor &gAccessor, GLTFType &type) {
    switch (gAccessor.type) {
        case TINYGLTF_TYPE_SCALAR:
            type = GLTFType::Scalar;
            break;
        case TINYGLTF_TYPE_VEC2:
            type = GLTFType::Vec2;
            break;
        case TINYGLTF_TYPE_VEC3:
            type = GLTFType::Vec3;
            break;
        case TINYGLTF_TYPE_VEC4:
            type = GLTFType::Vec4;
            break;
        case TINYGLTF_TYPE_MAT2:
            type = GLTFType::Mat2;
            break;
        case TINYGLTF_TYPE_MAT3:
            type = GLTFType::Mat3;
            break;
        case TINYGLTF_TYPE_MAT4:
            type = GLTFType::Mat4;
            break;
        default:
            perr("Unsupported Type was provided to the GLTF Loader!");
            return false;
    }
    return true;
}

bool VROGLTFLoader::getPrimitiveType(int mode, VROGeometryPrimitiveType &type) {
    switch (mode) {
        case TINYGLTF_MODE_LINE:
            type = VROGeometryPrimitiveType::Line;
            break;
        case TINYGLTF_MODE_POINTS:
            type = VROGeometryPrimitiveType::Point;
            break;
        case TINYGLTF_MODE_TRIANGLES:
            type = VROGeometryPrimitiveType::Triangle;
            break;
        case TINYGLTF_MODE_TRIANGLE_STRIP:
            type = VROGeometryPrimitiveType::TriangleStrip;
            break;
        case TINYGLTF_MODE_LINE_LOOP:
            // TODO VIRO-3686: Add support for additional primitive modes.
            perr("GLTF Mode LINE_LOOP primitive is not yet supported!");
            return false;
        case TINYGLTF_MODE_TRIANGLE_FAN:
            // TODO VIRO-3686: Add support for additional primitive modes.
            perr("GLTF Mode TRIANGLE_FAN primitive is not yet supported!");
            return false;
        default:
            perr("Unsupported GLTF primitive type provided for this model.");
            return false;
    }
    return true;
}

VROFilterMode VROGLTFLoader::getFilterMode(int mode) {
    switch(mode) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
            //choose 1 pixel from the biggest mip
            return VROFilterMode::Nearest;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
            //choose 4 pixels from the biggest mip and blend them
            return VROFilterMode ::Linear;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            // Behavior: Choose the best mip, then pick one pixel from that mip
            // TODO VIRO-3687: Add additional Sampler filter modes. Fallback to nearest for now.
            return VROFilterMode::Nearest;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            // Behavior: choose the best mip, then blend 4 pixels from that mip
            // TODO VIRO-3687: Add additional Sampler filter modes. Fallback to nearest for now.
            return VROFilterMode::Nearest;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            // Behavior: choose the best 2 mips, choose 1 pixel from each, blend them
            // TODO VIRO-3687: Add additional Sampler filter modes. Fallback to Linear for now.
            return VROFilterMode::Linear;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            // Behavior: choose the best 2 mips. choose 4 pixels from each, blend them
            // TODO VIRO-3687: Add additional Sampler filter modes. Fallback to Linear for now.
            return VROFilterMode::Linear;
        default:
            return VROFilterMode::Linear;
    }
}

VROWrapMode VROGLTFLoader::getWrappingMode(int mode) {
    switch(mode) {
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            return VROWrapMode ::ClampToBorder;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
            return VROWrapMode::Mirror;
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
            return VROWrapMode::Repeat;
        default:
            return VROWrapMode::Repeat;
    }
}

void VROGLTFLoader::loadGLTFFromResource(std::string gltfManifestFilePath, const std::map<std::string, std::string> overwriteResourceMap,
                                         VROResourceType resourceType, std::shared_ptr<VRONode> rootNode, bool isGLTFBinary,
                                         std::shared_ptr<VRODriver> driver, std::function<void(std::shared_ptr<VRONode>, bool)> onFinish) {
      // First, retrieve the main GLTF 'json manifest' file (either .gltf or .glb)
      VROModelIOUtil::retrieveResourceAsync(gltfManifestFilePath, resourceType,
                                            [gltfManifestFilePath, overwriteResourceMap, isGLTFBinary, rootNode, driver, onFinish]
                                            (std::string cachedFilePath, bool isTemp) {
                // Then use TinyGltf to parse the GTLF structure, and corresponding auxiliary resource files.
                VROPlatformDispatchAsyncBackground([gltfManifestFilePath, overwriteResourceMap, isGLTFBinary, cachedFilePath, rootNode, driver, onFinish] {
                    tinygltf::Model gModel;
                    tinygltf::TinyGLTF gLoader;
                    std::string err;

                    // If we've successfully retrieved the GLTF Manifest, start parsing the file with tinyGLTF.
                    bool ret = false;
                    if (isGLTFBinary) {
                        ret = gLoader.LoadBinaryFromFile(&gModel, &err, cachedFilePath, overwriteResourceMap);
                    } else {
                        ret = gLoader.LoadASCIIFromFile(&gModel, &err, cachedFilePath, gltfManifestFilePath, overwriteResourceMap);
                    }

                    // Fail fast if any errors were encountered.
                    if (!err.empty()) {
                        pwarn("Error when parsing GTLF manifest: %s", err.c_str());
                        onFinish(nullptr, false);
                        return;
                    }

                    if (!ret) {
                        pwarn("Failed to parse glTF manifest.");
                        onFinish(nullptr, false);
                        return;
                    }

                    // Ensure that we are only processing GLTF 2.0 models.
                    std::string version = gModel.asset.version;
                    if (VROStringUtil::toFloat(version) < 2) {
                        pwarn("Error parsing GLTF model: Only GLTF 2.0 models are supported!");
                        onFinish(nullptr, false);
                        return;
                    }

                    // Once the manifest has been parsed, start constructing our Viro 3D Model.
                    const tinygltf::Model &model = gModel;
                    VROPlatformDispatchAsyncRenderer([rootNode, model, driver, onFinish] {
                        _dataCache.clear();
                        _textureCache.clear();

                        bool success = true;
                        std::shared_ptr<VRONode> gltfRootNode = std::make_shared<VRONode>();
                        for (const tinygltf::Scene gScene : model.scenes) {
                            if (!processScene(model, gltfRootNode, gScene)) {
                                success = false;
                                break;
                            }
                        }

                        // Once we have processed the model, injected it into the scene.
                        injectGLTF(success ? gltfRootNode : nullptr, rootNode, driver, onFinish);

                        // Clean up our cached resources.
                        _dataCache.clear();
                        _textureCache.clear();
                    });
                });
            },
            [gltfManifestFilePath, rootNode, onFinish]() {
                // Else if we have failed to retrieve the GTLF file, notify delegates on failure
                perr("Failed to retrieve GTLF file: %s", gltfManifestFilePath.c_str());
                onFinish(rootNode, false);
            });
}

void VROGLTFLoader::injectGLTF(std::shared_ptr<VRONode> gltfNode,
                             std::shared_ptr<VRONode> rootNode,
                             std::shared_ptr<VRODriver> driver,
                             std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish) {
    if (gltfNode) {
        // The top-level fbxNode is a dummy; all of the data is stored in the children, so we
        // simply transfer those children over to the destination node
        for (std::shared_ptr<VRONode> child : gltfNode->getChildNodes()) {
            rootNode->addChildNode(child);
        }

        // Recompute the node's umbrellaBoundingBox and set the atomic rendering properties before
        // we notify the user that their FBX has finished loading
        rootNode->recomputeUmbrellaBoundingBox();
        rootNode->syncAppThreadProperties();
        rootNode->setIgnoreEventHandling(rootNode->getIgnoreEventHandling());

        // Hydrate the geometry and all textures prior to invoking the callback
        VROModelIOUtil::hydrateNodes(rootNode, driver);

        if (onFinish) {
            onFinish(rootNode, true);
        }
    }
    else {
        if (onFinish) {
            onFinish(rootNode, false);
        }
    }
}

bool VROGLTFLoader::processScene(const tinygltf::Model &gModel, std::shared_ptr<VRONode> rootNode, const tinygltf::Scene &gScene) {
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    sceneNode->setName(gScene.name);

    std::vector<int> gNodeIndexes = gScene.nodes;
    for (int gNodeIndex : gNodeIndexes) {
        // Fail fast if we have failed to process a node in the scene.
        if (!processNode(gModel, sceneNode, gNodeIndex)) {
            return false;
        }
    }

    rootNode->addChildNode(sceneNode);
    return true;
}

bool VROGLTFLoader::processNode(const tinygltf::Model &gModel, std::shared_ptr<VRONode> &parentNode, int gNodeIndex) {
    tinygltf::Node gNode = gModel.nodes[gNodeIndex];

    // Grab the main transformations for this node.
    std::vector<double> gScale = gNode.scale;
    std::vector<double> gPos = gNode.translation;
    std::vector<double> gRot = gNode.rotation;
    VROVector3f pos = gPos.size() > 0 ? VROVector3f(gPos[0], gPos[1], gPos[2]) : VROVector3f();
    VROVector3f scale = gScale.size() > 0 ? VROVector3f(gScale[0], gScale[1], gScale[2]) : VROVector3f(1,1,1);
    VROQuaternion rot = gRot.size() > 0 ? VROQuaternion(gRot[0], gRot[1], gRot[2], gRot[3]) : VROQuaternion();

    // If a transformation matrix is provided, we use that instead.
    std::vector<double> gMat = gNode.matrix;
    if (gMat.size() > 0) {
        std::vector<float> gFloatMatrix;
        for (int i = 0; i < 16; i ++) {
            gFloatMatrix.push_back((float)gMat[i]);
        }
        VROMatrix4f mat = VROMatrix4f(&gFloatMatrix[0]);
        pos = mat.extractTranslation();
        scale = mat.extractScale();
        rot = mat.extractRotation(scale);
    }

    // Finally set the parsed transforms on VRONode.
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    node->setPosition(pos);
    node->setScale(scale);
    node->setRotation(rot);

    // Process the Geometry for this node, if any.
    // Fail fast if we have failed to process the node's mesh.
    int meshIndex = gNode.mesh;
    if (meshIndex >= 0 && !processMesh(gModel, node, gModel.meshes[meshIndex])) {
        return false;
    }
    parentNode->addChildNode(node);

    // Recursively process each child node, if any.
    std::vector<int> gNodeChildrenIndexes = gNode.children;
    for (int gNodeIndex : gNodeChildrenIndexes) {
        // Fail fast if we have failed to process a node in the scene.
        if (!processNode(gModel, node, gNodeIndex)) {
            return false;
        }
    }
    return true;
}

bool VROGLTFLoader::processMesh(const tinygltf::Model &gModel, std::shared_ptr<VRONode> &rootNode, const tinygltf::Mesh &gMesh) {
    if (gMesh.primitives.size() <=0) {
        perr("GTLF requires mesh data to contain at least one primitive!");
        return false;
    }

    // Prepare our vec of elements (vertex indices) and sources (vertex attributes) to be parsed.
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;

    // A Single Mesh may contain more than one type of primitive type to be drawn. Cycle through them here.
    const std::vector<tinygltf::Primitive> &gPrimitives = gMesh.primitives;
    for (tinygltf::Primitive gPrimitive : gPrimitives) {

        // Grab Vertex indexing information needed for creating meshes.
        bool successVertex = processVertexElement(gModel, gPrimitive, elements);
        bool successAttributes  =processVertexAttributes(gModel, gPrimitive.attributes, sources, elements.size() - 1);
        if (!successVertex || !successAttributes) {
            pwarn("Failed to process mesh %s.", gMesh.name.c_str());
            return false;
        }

        // Grab the material pertaining to this primitive in this mesh.
        int gMatIndex = gPrimitive.material;
        if (gMatIndex >= 0) {
            const tinygltf::Material &gMat = gModel.materials[gMatIndex];
            materials.push_back(getMaterial(gModel, gMat));
        }
    }

    // Apply a default material if none has been specified:
    if (materials.size() == 0) {
        materials.push_back(std::make_shared<VROMaterial>());
    }

    // Finally construct our geometry with the processed vertex and attribute data.
    std::shared_ptr<VROGeometry> geometry = std::make_shared<VROGeometry>(sources, elements);
    geometry->setName(gMesh.name);
    rootNode->setGeometry(geometry);
    geometry->setMaterials(materials);
    return true;
}

bool VROGLTFLoader::processVertexElement(const tinygltf::Model &gModel,
                                         const tinygltf::Primitive &gPrimitive,
                                         std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    // Grab primitive type to be drawn for this geometry.
    VROGeometryPrimitiveType primitiveType;
    if (!getPrimitiveType(gPrimitive.mode, primitiveType)) {
        return false;
    }

    // Grab the vertex indices for this primitive.
    int gPimitiveIndicesIndex = gPrimitive.indices;
    if (gPimitiveIndicesIndex < 0) {
        // Fallback to glDrawArrays if no indexed vertices are provided.
        // TODO VIRO-3664: Support Draw Arrays for Viro Geometry in the main render pass.
        pwarn("Models requiring glDrawArray functionality are not yet supported.");
        return false;
    }

    // Grab the accessor that maps to a bufferView through which to view the data buffer
    // representing this geometry's indexed vertices.
    const tinygltf::Accessor &gIndiceAccessor = gModel.accessors[gPimitiveIndicesIndex];
    GLTFTypeComponent gTypeComponent;
    GLTFType gType;
    if (!getComponentType(gIndiceAccessor, gTypeComponent) || !getComponent(gIndiceAccessor, gType)) {
        return false;
    }

    // Warn here if the indexed vertex data is NOT of the proper type expected by GLTF for vertex indexing.
    if (gType != GLTFType::Scalar
        && gTypeComponent != GLTFTypeComponent::UnsignedByte
        && gTypeComponent != GLTFTypeComponent::UnsignedInt
        && gTypeComponent != GLTFTypeComponent::UnsignedShort
        && gTypeComponent != GLTFTypeComponent::Short) {
        perror("Unsupported Primitive type provided for GLTF vertex indexes.");
        return false;
    }

    // Calculate the byte stride size if none is provided from the BufferView.
    const tinygltf::BufferView &gIndiceBufferView = gModel.bufferViews[gIndiceAccessor.bufferView];
    size_t bufferViewStride = gIndiceBufferView.byteStride;
    if (bufferViewStride == 0) {
        int sizeOfSingleElement = (int) gType * (int) gTypeComponent;
        bufferViewStride = sizeOfSingleElement;
    }

    // Determine offsets and data sizes representing the indexed vertices's 'window of data' in the buffer
    int primitiveCount = VROGeometryUtilGetPrimitiveCount(gIndiceAccessor.count, primitiveType);
    size_t elementCount = gIndiceAccessor.count;
    size_t dataOffset = gIndiceAccessor.byteOffset + gIndiceBufferView.byteOffset;
    size_t dataLength = elementCount *  bufferViewStride;

    // Finally, grab the raw indexed vertex data from the buffer to be created with VROGeometryElement
    const tinygltf::Buffer &gBuffer = gModel.buffers[gIndiceBufferView.buffer];
    std::shared_ptr<VROData> data = std::make_shared<VROData>((void *)gBuffer.data.data(),
                                                              dataLength,
                                                              dataOffset);
    std::shared_ptr<VROGeometryElement> element
            = std::make_shared<VROGeometryElement>(data,
                                                   primitiveType,
                                                   primitiveCount,
                                                   (int) gTypeComponent);
    elements.push_back(element);
    return true;
}

bool VROGLTFLoader::processVertexAttributes(const tinygltf::Model &gModel,
                                            std::map<std::string, int> &gAttributes,
                                            std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                            size_t geoElementIndex) {

    // Iterate through each Vertex Attribute (Position / Normals / etc) to parse its data
    for (auto const& gAttribute : gAttributes) {
        std::string attributeName = gAttribute.first;
        int attributeAccessorIndex = gAttribute.second;

        // Ensure we only parse valid attributes, gracefully skip over the ones we do not yet support
        VROGeometrySourceSemantic attributeType = getGeometryAttribute(attributeName);
        if (attributeType == VROGeometrySourceSemantic::Invalid) {
            continue;
        }

        // Grab the accessor that maps to a bufferView through which to access the data buffer
        // representing this attribute's raw data.
        const tinygltf::Accessor &gAttributeAccesor = gModel.accessors[attributeAccessorIndex];
        GLTFTypeComponent gTypeComponent;
        GLTFType gType;
        if (!getComponentType(gAttributeAccesor, gTypeComponent) || !getComponent(gAttributeAccesor, gType)) {
            return false;
        }

        // Determine the offsets and data sizes representing the 'window of data' for this attribute in the buffer
        const tinygltf::BufferView gIndiceBufferView = gModel.bufferViews[gAttributeAccesor.bufferView];
        bool isFloat = gTypeComponent == GLTFTypeComponent::Float;
        size_t bufferViewOffset = gIndiceBufferView.byteOffset;
        size_t bufferViewTotalSize = gIndiceBufferView.byteLength;
        size_t attributeAccessorOffset = gAttributeAccesor.byteOffset;

        // Calculate the byte stride size if none is provided from the BufferView.
        size_t bufferViewStride = gIndiceBufferView.byteStride;
        if (bufferViewStride == 0) {
            int sizeOfSingleElement = (int) gType * (int) gTypeComponent;
            bufferViewStride = sizeOfSingleElement;
        }

        // Process and cache the attribute data to be used by the VROGeometrySource associated with
        // this attribute. If we've already been processed (cached) it before, simply grab it.
        std::string key = VROStringUtil::toString(gAttributeAccesor.bufferView);
        if (VROGLTFLoader::_dataCache.find(VROStringUtil::toString(gAttributeAccesor.bufferView)) == VROGLTFLoader::_dataCache.end()) {
            const tinygltf::Buffer &gbuffer = gModel.buffers[gIndiceBufferView.buffer];
            void *rawData = (void*)gbuffer.data.data();
            VROGLTFLoader::_dataCache[key] = std::make_shared<VROData>(rawData, bufferViewTotalSize, bufferViewOffset);
        }

        // Finally, build the Geometry source.
        int elementCount = gAttributeAccesor.count;
        std::shared_ptr<VROData> data = VROGLTFLoader::_dataCache[key];
        std::shared_ptr<VROGeometrySource> source = std::make_shared<VROGeometrySource>(data,
                                                                                        attributeType,
                                                                                        elementCount,
                                                                                        isFloat,
                                                                                        (int) gType,
                                                                                        (int) gTypeComponent,
                                                                                        attributeAccessorOffset,
                                                                                        bufferViewStride);

        // Because GLTF can have VROGeometryElements that corresponds to different sets of VROGeometrySources,
        // the render needs to be notified of this element-to-source mapping, so that the correct vertex
        // indices are bounded to and pointed by the right set of vertex attributes. Thus, we set the
        // correspondingly mapped VROGeometryElement index here to preserve that mapping.
        source->setGeometryElementIndex(geoElementIndex);
        sources.push_back(source);
    }

    return true;
}

std::shared_ptr<VROMaterial> VROGLTFLoader::getMaterial(const tinygltf::Model &gModel, const tinygltf::Material &gMat) {
    std::shared_ptr<VROMaterial> vroMat = std::make_shared<VROMaterial>();
    tinygltf::ParameterMap gAdditionalMap = gMat.additionalValues;

    // Process PBR values from the given tinyGLTF material into our VROMaterial, if any.
    processPBR(gModel, vroMat, gMat);

    // Process Normal textures
    std::shared_ptr<VROTexture> normalTexture = getTexture(gModel,gAdditionalMap, "normalTexture", false);
    if (normalTexture != nullptr) {
        vroMat->getNormal().setTexture(normalTexture);
    }

    // Process Occlusion Textures
    std::shared_ptr<VROTexture> occlusionTexture = getTexture(gModel,gAdditionalMap, "occlusionTexture", false);
    if (occlusionTexture != nullptr) {
        vroMat->getAmbientOcclusion().setTexture(occlusionTexture);
    }

    // Process GLTF transparency modes
    std::string mode = "OPAQUE";
    if (gAdditionalMap.find("alphaMode") != gAdditionalMap.end()) {
        mode = gAdditionalMap["alphaMode"].string_value;
    }

    if (VROStringUtil::strcmpinsensitive(mode, "OPAQUE")) {
        vroMat->setTransparencyMode(VROTransparencyMode::RGBZero);
    } else if (VROStringUtil::strcmpinsensitive(mode, "MASK")) {
        // TODO VIRO-3684: Implement Transparent Masks, fallback to Alpha transparency for now.
        vroMat->setTransparencyMode(VROTransparencyMode::AOne);
    } else if (VROStringUtil::strcmpinsensitive(mode, "BLEND")) {
        vroMat->setTransparencyMode(VROTransparencyMode::AOne);
    }

    // TODO VIRO-3683: Implement GLTF Emissive Maps
    vroMat->setName(gMat.name);
    return vroMat;
}

void VROGLTFLoader::processPBR(const tinygltf::Model &gModel, std::shared_ptr<VROMaterial> &vroMat, const tinygltf::Material &gMat) {
    std::map<std::string, tinygltf::Parameter> gPbrMap = gMat.pbrValues;
    if (gPbrMap.size() <= 0) {
        vroMat->setLightingModel(VROLightingModel::Lambert);
        return;
    }

    // Process PBR base color, defaults to white.
    VROVector4f color = VROVector4f(1, 1, 1, 1);
    if (gPbrMap.find("baseColorFactor") != gPbrMap.end()) {
        std::array<double, 4> vec4_color = gPbrMap["baseColorFactor"].ColorFactor();
        color = VROVector4f(vec4_color[0], vec4_color[1], vec4_color[2], vec4_color[3]);
    }
    vroMat->getDiffuse().setColor(color);

    // Process PBR metal factor, defaults to fully metallic.
    double metallicFactor = 1;
    if (gPbrMap.find("metallicFactor") != gPbrMap.end()) {
        metallicFactor = gPbrMap["metallicFactor"].Factor();
    }
    vroMat->getMetalness().setIntensity(metallicFactor);

    // Process PBR roughness factor, defaults to fully rough.
    double roughnessFactor = 1;
    if (gPbrMap.find("roughnessFactor") != gPbrMap.end()) {
        roughnessFactor = gPbrMap["roughnessFactor"].Factor();
    }
    vroMat->getRoughness().setIntensity(roughnessFactor);

    // Process a metallic / roughness texture if any, where metalness values are sampled from the
    // B channel and roughness values are sampled from the G channel.
    std::shared_ptr<VROTexture> metallicRoughnessTexture = getTexture(gModel, gPbrMap, "metallicRoughnessTexture", false);
    if (metallicRoughnessTexture != nullptr) {
        vroMat->getMetalness().setTexture(metallicRoughnessTexture);
        vroMat->getRoughness().setTexture(metallicRoughnessTexture);
    }

    // Grab the base color texture in sRGB space.
    std::shared_ptr<VROTexture> baseColorTexture = getTexture(gModel, gPbrMap, "baseColorTexture", true);
    if (baseColorTexture != nullptr) {
        vroMat->getDiffuse().setTexture(baseColorTexture);
    }
    vroMat->setLightingModel(VROLightingModel::PhysicallyBased);
}

std::shared_ptr<VROTexture> VROGLTFLoader::getTexture(const tinygltf::Model &gModel, std::map<std::string, tinygltf::Parameter> gPropMap,
                                                      std::string targetedTextureName, bool srgb) {
    if (gPropMap.find(targetedTextureName) == gPropMap.end()) {
        return nullptr;
    }

    int index = gPropMap[targetedTextureName].TextureIndex();
    if (index < 0) {
        return nullptr;
    }

    tinygltf::Texture gTexture = gModel.textures[index];
    return getTexture(gModel, gTexture, srgb);
}

std::shared_ptr<VROTexture> VROGLTFLoader::getTexture(const tinygltf::Model &gModel, const tinygltf::Texture &gTexture, bool srgb){
    std::shared_ptr<VROTexture> texture = nullptr;
    int imageIndex = gTexture.source;
    if (imageIndex < 0){
        perr("Attempted to grab an invalid GTLF texture source.");
        return nullptr;
    }

    // Return the texture if we had already previously processed and cached this image.
    if (VROGLTFLoader::_textureCache.find(VROStringUtil::toString(imageIndex)) != VROGLTFLoader::_textureCache.end()){
        std::string key = VROStringUtil::toString(imageIndex);
        return VROGLTFLoader::_textureCache[key];
    }

    // Grab the GLTF image data of for this texture.
    tinygltf::Image gImg = gModel.images[imageIndex];
    std::string imgName = gImg.name;

    // Decode the GLTF image data / raw bytes into a VROImage data.
    std::vector<unsigned char> data = gImg.rawByteVec;
    std::shared_ptr<VROImage> image = VROPlatformLoadImageWithBufferedData(data, VROTextureInternalFormat::RGBA8);
    if (image == nullptr){
        perr("Error when parsing texture for image %s.", imgName.c_str());
        return nullptr;
    }

    // Use the VROImage data to create a VROTexture, with parsed GLTF Sampler properties.
    texture = std::make_shared<VROTexture>(srgb, VROMipmapMode::Runtime, image);
    int samplerIndex = gTexture.sampler;
    if (samplerIndex >=0) {
        tinygltf::Sampler sampler = gModel.samplers[samplerIndex];
        texture->setWrapS(getWrappingMode(sampler.wrapS));
        texture->setWrapT(getWrappingMode(sampler.wrapT));
        texture->setMagnificationFilter(getFilterMode(sampler.magFilter));
        texture->setMinificationFilter(getFilterMode(sampler.minFilter));
    } else {
        texture->setWrapS(VROWrapMode::Repeat);
        texture->setWrapT(VROWrapMode::Repeat);
        texture->setMagnificationFilter(VROFilterMode::Linear);
        texture->setMinificationFilter(VROFilterMode::Linear);
    }

    // Cache a copy of the created texture as other elements may also refer to it.
    std::string key = VROStringUtil::toString(imageIndex);
    VROGLTFLoader::_textureCache[key] = texture;
    return texture;
}
