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
#include "VROBone.h"
#include "VROKeyframeAnimation.h"
#include "VROSkeletalAnimation.h"
#include "VROSkeleton.h"
#include "VROBoneUBO.h"
#include "VROLog.h"
#include "VROShaderFactory.h"
#include "VROShaderModifier.h"
#include "VROShaderProgram.h"
#include "VROMorpher.h"

static std::string kVROGLTFInputSamplerKey = "timeInput";
std::map<std::string, std::shared_ptr<VROVertexBuffer>> VROGLTFLoader::_dataCache;
std::map<std::string, std::shared_ptr<VROTexture>> VROGLTFLoader::_textureCache;
std::map<int, std::shared_ptr<VROSkeleton>> VROGLTFLoader::_skinIndexToSkeleton;
std::map<int, std::map<int,int>> VROGLTFLoader::_skinIndexToJointNodeIndex;
std::map<int, std::map<int,std::vector<int>>> VROGLTFLoader::_skinIndexToJointChildJoints;
std::map<int, std::map<int, std::vector<std::shared_ptr<VROKeyframeAnimation>>>> VROGLTFLoader::_nodeKeyFrameAnims;
std::map<int, std::vector<std::shared_ptr<VROSkeletalAnimation>>> VROGLTFLoader::_skinSkeletalAnims;
std::map<int, std::shared_ptr<VROSkinner>> VROGLTFLoader::_skinMap;

static int getTypeSize(GLTFType type) {
    switch (type) {
        case GLTFType::Scalar: return 1;
        case GLTFType::Vec2: return 2;
        case GLTFType::Vec3: return 3;
        case GLTFType::Vec4: return 4;
        case GLTFType::Mat2: return 4;
        case GLTFType::Mat3: return 9;
        case GLTFType::Mat4: return 16;
        default: pabort();
    }
}

static int getComponentTypeSize(GLTFTypeComponent type) {
    switch (type) {
        case GLTFTypeComponent::Byte: return 1;
        case GLTFTypeComponent::UnsignedByte: return 1;
        case GLTFTypeComponent::Short: return 2;
        case GLTFTypeComponent::UnsignedShort: return 2;
        case GLTFTypeComponent::UnsignedInt: return 4;
        case GLTFTypeComponent::Float: return 4;
        default: pabort();
    }
}

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
        return VROGeometrySourceSemantic::BoneIndices;
    } else if (VROStringUtil::strcmpinsensitive(name, "WEIGHTS_0")) {
        return VROGeometrySourceSemantic::BoneWeights;
    } else {
        pwarn("Attempted to parse an unknown geometry attribute: %s", name.c_str());
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
                        clearCachedData();

                        // Process and cache skinner and skeletal data needed for skeletal animation
                        // and skinner geometry to be set later on our nodes.
                        if (!processSkinner(model)) {
                            perr("Error when processing the skinner of GLTF model!");
                            onFinish(nullptr, false);
                            return;
                        }

                        // Now generate our KeyFrame and skeletal animations and cache them to be
                        // set later on our nodes (when we iterate through the scene hierarchy).
                        if (!processAnimations(model)) {
                            pwarn("Error when processing animation data of the GLTF model!");
                            onFinish(nullptr, false);
                            return;
                        }

                        // Finally, iterate through gLTF model data and build out our VRONodes that
                        // represent our 3D Model scene, setting cached animations / skinners on
                        // those nodes along the way.
                        bool success = true;
                        std::shared_ptr<VRONode> gltfRootNode = std::make_shared<VRONode>();
                        for (const tinygltf::Scene gScene : model.scenes) {
                            if (!processScene(model, gltfRootNode, gScene, driver)) {
                                success = false;
                                break;
                            }
                        }

                        for (auto &skeletonPair : _skinIndexToSkeleton) {
                            std::shared_ptr<VROSkinner> skin = _skinMap[skeletonPair.first];
                            skeletonPair.second->setSkinnerRootNode(skin->getSkinnerNode());
                        }

                        // Once we have processed the model, injected it into the scene.
                        injectGLTF(success ? gltfRootNode : nullptr, rootNode, driver, onFinish);

                        // Clean up our cached resources.
                        clearCachedData();
                    });
                });
            },
            [gltfManifestFilePath, rootNode, onFinish]() {
                // Else if we have failed to retrieve the GTLF file, notify delegates on failure
                perr("Failed to retrieve GTLF file: %s", gltfManifestFilePath.c_str());
                onFinish(rootNode, false);
            });
}

void VROGLTFLoader::clearCachedData() {
    _dataCache.clear();
    _textureCache.clear();
    _skinIndexToSkeleton.clear();
    _skinIndexToJointNodeIndex.clear();
    _skinIndexToJointChildJoints.clear();
    _skinMap.clear();
    _nodeKeyFrameAnims.clear();
    _skinSkeletalAnims.clear();
}

bool VROGLTFLoader::processSkinner(const tinygltf::Model &model) {
    if (model.skins.size() == 0) {
        return true;
    }

    // Create a map of all known joints used for building a tree of the
    // skeletal bones in this model.
    std::map<int, std::map<int,int>> skinIndexToNodeJointIndexes;
    for (int skinIndex = 0; skinIndex < model.skins.size(); skinIndex ++) {
        tinygltf::Skin skin = model.skins[skinIndex];

        for (int jointIndex = 0; jointIndex < skin.joints.size(); jointIndex++) {
            int nodeIndexOfJoint = skin.joints[jointIndex];
            skinIndexToNodeJointIndexes[skinIndex][nodeIndexOfJoint] = jointIndex;
            _skinIndexToJointNodeIndex[skinIndex][jointIndex] = nodeIndexOfJoint;
        }
    }

    // Construct _skinIndexToJointChildJoints and skinIndexToJointParentJoint mappings
    // respectively - these effectively maps out the joints within the skeletal tree
    // by keeping track of a joint's pointer to both child and parent joints.
    std::map<int, std::map<int, int>> skinIndexToJointParentJoint;
    for (int skinIndex = 0; skinIndex < model.skins.size(); skinIndex ++) {
        tinygltf::Skin skin = model.skins[skinIndex];
        for (int jointIndex = 0; jointIndex < skin.joints.size(); jointIndex++) {

            // Grab the node index corresponding to this joint.
            int nodeIndex = _skinIndexToJointNodeIndex[skinIndex][jointIndex];

            // Then look up the node with the nodeIndex and grab it's child nodes.
            std::vector<int> childrenNodeIndex = model.nodes[nodeIndex].children;
            for (int childIndex : childrenNodeIndex) {
                // Skip if we have child indexes that are not a joint.
                if (skinIndexToNodeJointIndexes[skinIndex].find(childIndex) == skinIndexToNodeJointIndexes[skinIndex].end()) {
                    continue;
                }

                // For each of the child node index, get it's corresponding joint index.
                int subJointIndex = skinIndexToNodeJointIndexes[skinIndex][childIndex];

                // Parent the all childed joint with it's parent. Also create a reverse map
                // in skinIndexToJointParentJoint.
                _skinIndexToJointChildJoints[skinIndex][jointIndex].push_back(subJointIndex);
                skinIndexToJointParentJoint[skinIndex][subJointIndex] = jointIndex;
            }
        }
    }

    // Finally, use the joint tree to construct our VROSkeleton and VROSkinners.
    for (int skinIndex = 0; skinIndex < model.skins.size(); skinIndex ++) {
        tinygltf::Skin skin = model.skins[skinIndex];

        // Process and cache a copy of a VROSkinner.
        std::vector<VROMatrix4f> invBindTransformsOut;
        if (!processSkinnerInverseBindData(model, model.skins[skinIndex], invBindTransformsOut)) {
            pwarn("Failed to process Skinner");
            return false;
        }

        std::vector<std::shared_ptr<VROBone>> bones;
        for (int jointIndex = 0; jointIndex < skin.joints.size(); jointIndex++) {
            int parentJointIndex = skinIndexToJointParentJoint[skinIndex][jointIndex];
            
            // TODO We need the bone local transform if we want layered animations to work with GLTF
            VROMatrix4f boneSpaceBindTransform = invBindTransformsOut[jointIndex];
            std::string name = "BoneIndex_" + VROStringUtil::toString(parentJointIndex);
            std::shared_ptr<VROBone> bone = std::make_shared<VROBone>(jointIndex,
                                                                      parentJointIndex,
                                                                      name,
                                                                      VROMatrix4f::identity(),
                                                                      boneSpaceBindTransform);
            bones.push_back(bone);
        }

        std::shared_ptr<VROSkeleton> skeleton = std::make_shared<VROSkeleton>(bones);
        _skinIndexToSkeleton[skinIndex] = skeleton;
        _skinMap[skinIndex] = std::shared_ptr<VROSkinner>(new VROSkinner(skeleton,
                                                   VROMatrix4f(),
                                                   invBindTransformsOut,
                                                   nullptr,
                                                   nullptr));
    }
    return true;
}

bool VROGLTFLoader::processAnimations(const tinygltf::Model &model) {
    if (model.animations.size() == 0) {
        return true;
    }

    // In gLTF, model.animations represents a mixed mixed group of keyframe animation and
    // skeletal animation data corresponding to different nodes with different input times.
    // Thus, we need to group gLTF animation data for easier parsing. We group as follows:
    //
    // - Group all animations that apply only to it's corresponding node.
    // - Then group a node's set of animation data by animation name or id.
    // - Then group the single animation's data with by same input time frame / inputIndex.
    // - Data is then stored and referred to as channelIndex.
    //
    // This results in:
    // <nodeIndex, <animationIndex , <inputIndex, vec<channelIndex>>>> nodeToAnimDataMap;
    //
    std::map<int, std::map<int, std::map<int, std::vector<int>>>> nodeToAnimDataMap;
    std::map<int, std::set<int>> animToNodeIndexMap;
    for (int animIndex = 0; animIndex < model.animations.size(); animIndex++) {
        tinygltf::Animation anim = model.animations[animIndex];

        // For each animation, iterate through the channel input data.
        for (int cIndex = 0; cIndex < anim.channels.size(); cIndex++) {
            tinygltf::AnimationChannel channel = anim.channels[cIndex];

            // Group all the channel input data with the same matching sampler inputAcessor
            // index (ensures same keyframe data).
            int channelInputIndex = anim.samplers[channel.sampler].input;
            nodeToAnimDataMap[channel.target_node][animIndex][channelInputIndex].push_back(cIndex);

            // Also save a reverse map of anim to nodeIndexes for skeletal animation processing.
            animToNodeIndexMap[animIndex].insert(channel.target_node);
        }
    }

    // Group all the node indexes associated with a given skin to be used for
    // skeletal animation processing.
    std::map<int, std::set<int>> skinToNodeMap;
    for (int skinIndex = 0; skinIndex < model.skins.size(); skinIndex ++) {
        tinygltf::Skin skin = model.skins[skinIndex];
        for (int jointIndex = 0; jointIndex < skin.joints.size(); jointIndex++) {
            skinToNodeMap[skinIndex].insert(skin.joints[jointIndex]);
        }
    }

    // For each animation, determine if it is a skeletalAnimation (gLTF doesn't tell us)
    // To do this, we assume that if an animation consists of all of the nodes specified in a
    // skinner, it is treated automatically as a skeletalAnim.
    std::vector<std::pair<int,int>> skeletalAnimToSkinPair;
    for (int animIndex = 0; animIndex < animToNodeIndexMap.size(); animIndex++) {
        for (int skinIndex = 0; skinIndex < skinToNodeMap.size(); skinIndex ++) {
            if (animToNodeIndexMap[animIndex] == skinToNodeMap[skinIndex]) {
                skeletalAnimToSkinPair.push_back(std::make_pair(animIndex, skinIndex));
            }
        }
    }

    // Finally, process all animations after grouping the data above.
    if (!processAnimationKeyFrame(model, nodeToAnimDataMap)) {
        perr("Error when parsing key frame animations");
        return false;
    }

    processSkeletalAnimation(model, skeletalAnimToSkinPair);
    return true;
}

bool VROGLTFLoader::processAnimationKeyFrame(const tinygltf::Model &model,
                                             std::map<int, std::map<int, std::map<int, std::vector<int>>>> &animatedNodes) {
    // Iterate through all animations that are mapped to a node in nodeIndex within
    // animatedNodes. For each animation, process its data via animation channels
    // and create a VROKeyframeAnimation from it. Cache the results in _nodeKeyFrameAnim.
    for (auto const& animNode : animatedNodes) {

        // Grab grouped animations of the form: <animationIndex , <inputIndex, vec<channelIndex>>>
        std::map<int, std::map<int, std::vector<int>>> animations = animNode.second;
        int nodeIndex = animNode.first;

        for (auto const &anim : animations) {
            // A single animation for a Node should only have one 'time' input sampler
            // that effectively outlines the entire duration of that animation.
            // Else, it is ambiguous as to the order of multiple input samplers.
            // Thus, perform a sanity check here to ensure we only have one.
            int animationIndex = anim.first;
            std::map<int, std::vector<int>> channelData = anim.second;

            // In some models, for the same animation, we can have multiple time input
            // sampler data. Although strange, this would only hold true if we want to
            // animate separate animation properties at different interval speeds towards
            // the same end duration. For example, a translation in a different speed than
            // rotation, for the same node, for the same animation.
            if (channelData.size() > 1) {
                pwarn("Multiple time input samplers detected for a single animation.");
            }

            for (auto inputSamplerToAnimationsPair : channelData) {
                // Create a VROKeyframeAnimation with the targeted channel.
                std::shared_ptr<VROKeyframeAnimation> animation = nullptr;
                if (!processAnimationChannels(model,
                                              model.animations[animationIndex],
                                              inputSamplerToAnimationsPair.second,
                                              animation)) {
                    return false;
                }
                
                // If no animation is given, set a default one with an index reference.
                std::string name = model.animations[anim.first].name;
                if (name.size() == 0) {
                    name = "animation_" + VROStringUtil::toString(animationIndex);
                }
                animation->setName(name);
                
                float duration = animation->getDuration();
                for (auto currAnim : _nodeKeyFrameAnims[nodeIndex][anim.first]) {
                    if (duration != currAnim->getDuration()) {
                        perr("Detected mis-matching durations for animation properties.");
                        return false;
                    }
                }
                
                _nodeKeyFrameAnims[nodeIndex][anim.first].push_back(animation);
            }
        }
    }
    return true;
}

bool VROGLTFLoader::processAnimationChannels(const tinygltf::Model &gModel,
                                             const tinygltf::Animation &anim,
                                             std::vector<int> targetedChannelIndexes,
                                             std::shared_ptr<VROKeyframeAnimation> &animOut) {
    if (targetedChannelIndexes.size() <= 0) {
        pwarn("Attempted to process an invalid gLTF animation.");
        return false;
    }

    // Process input key frames first to create our vector of KeyFrameAnimationFrames to populate
    // with animation data like Position, Rotation, Scaling.
    std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> frames;
    tinygltf::AnimationChannel inputChannel = anim.channels[targetedChannelIndexes.front()];
    tinygltf::AnimationSampler inputSampler = anim.samplers[inputChannel.sampler];
    if (!processRawChannelData(gModel, kVROGLTFInputSamplerKey, inputChannel.target_node, inputSampler, frames)) {
        return false;
    }

    if (frames.size() == 0) {
        perror("Unable to properly parse Animation frames");
        return false;
    }

    // Set the total duration of this keyframe animation, in seconds
    float duration = frames.back()->time;

    // Normalize the input key frame time (expected by Viro's animation system)
    for (int i = 0; i < frames.size(); i++) {
        frames[i]->time = frames[i]->time / duration;
    }

    // Grab the channel object from the given index, and process it's raw buffered data
    // into translation, scale or rotation for this keyframe animation.
    bool hasTranslation = false;
    bool hasRotation = false;
    bool hasScale = false;
    bool hasMorphWeights = false;
    for (int channelIndex : targetedChannelIndexes) {
        tinygltf::AnimationChannel channel = anim.channels[channelIndex];
        tinygltf::AnimationSampler gSampler = anim.samplers[channel.sampler];
        if (!processRawChannelData(gModel, channel.target_path, channel.target_node, gSampler, frames)) {
            perr("Failed to process channel index %s for gltf model!", channel.target_path.c_str());
            return false;
        }

        if (VROStringUtil::strcmpinsensitive(channel.target_path, "translation")) {
            hasTranslation = true;
        } else if (VROStringUtil::strcmpinsensitive(channel.target_path, "scale")) {
            hasScale = true;
        } else if (VROStringUtil::strcmpinsensitive(channel.target_path, "rotation")) {
            hasRotation = true;
        }  else if (VROStringUtil::strcmpinsensitive(channel.target_path, "weights")) {
            hasMorphWeights = true;
        }
    }

    animOut = std::make_shared<VROKeyframeAnimation>(frames, duration, hasTranslation, hasRotation, hasScale, hasMorphWeights);
    return true;
}

bool VROGLTFLoader::processRawChannelData(const tinygltf::Model &gModel,
                                          std::string channelProperty,
                                          int channelTarget,
                                          const tinygltf::AnimationSampler &channelSampler,
                                          std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> &framesOut) {
    // TODO VIRO-3848: Support additional 3D Model Interpolation types
    if (!VROStringUtil::strcmpinsensitive(channelSampler.interpolation, "linear")) {
        pwarn("Viro does not currently support non-linear animations for gltf models.");
        return false;
    }

    // Grab the accessor that maps to a bufferView through which to access the data buffer
    // representing this channel's raw data.
    int accessorIndex = channelSampler.output;
    if (VROStringUtil::strcmpinsensitive(channelProperty, kVROGLTFInputSamplerKey)) {
        accessorIndex = channelSampler.input;
    }
    const tinygltf::Accessor &gDataAcessor = gModel.accessors[accessorIndex];
    GLTFTypeComponent gTypeComponent;
    GLTFType gType;
    if (!getComponentType(gDataAcessor, gTypeComponent) || !getComponent(gDataAcessor, gType)) {
        perr("Invalid Animation channel data provided!");
        return false;
    }

    // Since GLTF does not tell us which primitive the morph targets belong to,
    // we always grab the targets of the first primitive from the target node's mesh.
    int numberOfMorphTargets = 0;
    if (VROStringUtil::strcmpinsensitive(channelProperty, "weights")) {
        int meshIndex = gModel.nodes[channelTarget].mesh;
        if (gModel.meshes[meshIndex].primitives.size() <= 0) {
            perr("Missing morph targets needed for animation!");
            return false;
        }
        
        // Always assume that all primitives have the same amount of targets.
        numberOfMorphTargets = gModel.meshes[meshIndex].primitives.front().targets.size();
    }

    // Calculate the byte stride size if none is provided from the BufferView.
    const tinygltf::BufferView &gIndiceBufferView = gModel.bufferViews[gDataAcessor.bufferView];
    size_t bufferViewStride = gIndiceBufferView.byteStride;
    if (bufferViewStride == 0) {
        int sizeOfSingleElement = getTypeSize(gType) * getComponentTypeSize(gTypeComponent);
        bufferViewStride = sizeOfSingleElement;
    }

    // Determine offsets and data sizes representing the 'window of data' in the buffer
    size_t elementCount = gDataAcessor.count;
    size_t dataOffset = gDataAcessor.byteOffset + gIndiceBufferView.byteOffset;
    size_t dataLength = elementCount * bufferViewStride;

    if (VROStringUtil::strcmpinsensitive(channelProperty, kVROGLTFInputSamplerKey)) {
        // A kVROGLTFInputSamplerKey channelProperty param is provided when creating new, empty
        // VROKeyFrameAnimations to be used for storing animation data.
        for (int i = 0; i < elementCount; i ++) {
            std::unique_ptr<VROKeyframeAnimationFrame> frame
                    = std::unique_ptr<VROKeyframeAnimationFrame>(new VROKeyframeAnimationFrame());
            framesOut.push_back(std::move(frame));
        }
    } else if (VROStringUtil::strcmpinsensitive(channelProperty, "weights")) {
        if ((elementCount != (framesOut.size() * numberOfMorphTargets))) {
            perr("Animation frame %s do not match number of morph keyframes %d, %d",
                 channelProperty.c_str(), elementCount, framesOut.size() * numberOfMorphTargets);
            return false;
        }
    } else if (elementCount != framesOut.size()) {
        // Else if we are processing an output channel (animation data relating to key frames),
        // ensure that the size of the data aligns with the number of expected key frames.
        perr("Animation frame %s do not match number of keyframes %d, %d", channelProperty.c_str(), elementCount, framesOut.size());
        return false;
    }

    // Now process that buffer to produce the right output data.
    const tinygltf::Buffer &gBuffer = gModel.buffers[gIndiceBufferView.buffer];
    std::vector<float> tempVec;
    int morphIndex = 0;
    VROByteBuffer buffer((char *)gBuffer.data.data() + dataOffset, dataLength, false);
    for (int elementIndex = 0; elementIndex < elementCount; elementIndex++) {

        // Set the buffer position to begin at each element index - Ex: Each Vec4.
        buffer.setPosition(elementIndex * bufferViewStride);
        tempVec.clear();

        // For the current element, cycle through each of its float or type component
        // and convert them into a float through the math conversions required by gLTF.
        for (int componentCount = 0; componentCount < getTypeSize(gType); componentCount ++) {
            if (gTypeComponent == GLTFTypeComponent::Float) {
                float floatData = buffer.readFloat();
                tempVec.push_back(floatData);
            } else if (gTypeComponent == GLTFTypeComponent::UnsignedShort) {
                unsigned short uShortData = buffer.readUnsignedShort();
                float point = uShortData / 65535.0;
                tempVec.push_back(point);
            } else if (gTypeComponent == GLTFTypeComponent::Short) {
                short shortData = buffer.readShort();
                float point = std::max(shortData / 32767.0, -1.0);
                tempVec.push_back(point);
            } else if (gTypeComponent == GLTFTypeComponent::UnsignedByte) {
                unsigned uByteData = buffer.readUnsignedByte();
                float point = uByteData / 255.0;
                tempVec.push_back(point);
            } else if (gTypeComponent == GLTFTypeComponent::Byte) {
                signed char byteData = buffer.readByte();
                float point = std::max(byteData / 127.0, -1.0);
                tempVec.push_back(point);
            } else {
                pwarn("Invalid element type in Animation Accessor Data: %d", gTypeComponent);
                return false;
            }
        }

        if (VROStringUtil::strcmpinsensitive(channelProperty, "translation") && tempVec.size() == 3) {
            framesOut[elementIndex]->translation = {tempVec[0], tempVec[1], tempVec[2]};
        } else if (VROStringUtil::strcmpinsensitive(channelProperty, "rotation")&& tempVec.size() == 4) {
            framesOut[elementIndex]->rotation = {tempVec[0], tempVec[1], tempVec[2], tempVec[3]};
        } else if (VROStringUtil::strcmpinsensitive(channelProperty, "scale")&& tempVec.size() == 3) {
            framesOut[elementIndex]->scale = {tempVec[0], tempVec[1], tempVec[2]};
        } else if (VROStringUtil::strcmpinsensitive(channelProperty, kVROGLTFInputSamplerKey)) {
            framesOut[elementIndex]->time = tempVec[0];
        } else if (VROStringUtil::strcmpinsensitive(channelProperty, "weights")) {
            // GLTF only supports animating one primitive's morph target on a node at a time
            // and as such we process only a single morph target per channel.
            int meshIndex = gModel.nodes[channelTarget].mesh;
            std::string key = getMorphTargetName(gModel,
                                                 gModel.meshes[meshIndex].primitives.front(),
                                                 morphIndex);
            framesOut[elementIndex/numberOfMorphTargets]->morphWeights[key] = tempVec[0];

            // The weights for all morph targets are tightly interleaved. As such, use
            // morphIndex to increment, track and accumulate all morph data for this frame.
            // Once we have hit the numberOfMorphTargets, we reset this to 0 to move
            // onto the next frame containing the new set of morph data.
            morphIndex ++;
            if (morphIndex >= numberOfMorphTargets) {
                morphIndex = 0;
            }
        } else {
            pwarn("Invalid target path %s with data size %d provided for gLTF.", channelProperty.c_str(), (int) tempVec.size());
            return false;
        }
    }
    return true;
}

void VROGLTFLoader::processSkeletalAnimation(const tinygltf::Model &model,
                                     std::vector<std::pair<int,int>> &skeletalAnimToSkinPair) {
    if (skeletalAnimToSkinPair.size() == 0) {
        return;
    }

    // Process any skeletal animation that is associated with each skinner in the scene.
    for (int i = 0; i < skeletalAnimToSkinPair.size(); i++) {
        int skinIndex = skeletalAnimToSkinPair[i].second;
        int skeletalAnimationIndex = skeletalAnimToSkinPair[i].first;

        // Create a set of skeletal Frames, populate them with empty key frames
        std::vector<std::unique_ptr<VROSkeletalAnimationFrame>> skeletalFrames;
        int firstNodeIndex = _skinIndexToJointNodeIndex[skinIndex][0];
        std::vector<std::shared_ptr<VROKeyframeAnimation>> keyFrameAnims = _nodeKeyFrameAnims[firstNodeIndex][skeletalAnimationIndex];

        // TODO: Support mutli-layered parallel keyframe animations.
        std::shared_ptr<VROKeyframeAnimation> keyFrameAnim = keyFrameAnims.front();
        float totalDuration = keyFrameAnim->getDuration();

        const std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> &frames = keyFrameAnim->getFrames();
        for (int i = 0; i < frames.size(); i++) {
            std::unique_ptr<VROSkeletalAnimationFrame> skeletalFrame = std::unique_ptr<VROSkeletalAnimationFrame>(new VROSkeletalAnimationFrame());
            skeletalFrame->time = frames[i]->time;
            
            // TODO We should support the non-legacy (concatenated transform) format
            skeletalFrame->boneTransformsLegacy = true;
            skeletalFrames.push_back(std::move(skeletalFrame));
        }

        // Now iterate through each frame and grab the computed transform for each joint/bone.
        std::shared_ptr<VROSkeleton> currentSkeleton = _skinIndexToSkeleton[skinIndex];
        std::shared_ptr<VROSkinner> currentSkinner = _skinMap[skinIndex];
        for (int i = 0; i < frames.size(); i++) {
            std::map<int, VROMatrix4f> computedAnimatedJointTrans;
            processSkeletalTransformsForFrame(skinIndex, skeletalAnimationIndex, i, 0,
                                              computedAnimatedJointTrans);

            // Then, for each joint, move the transform the computed joint back into bone space
            // and save that into the skeletalFrames for the animation.
            for (int jointI = 0; jointI < computedAnimatedJointTrans.size(); jointI++) {
                VROMatrix4f invBind = _skinMap[skinIndex]->getBindTransforms()[jointI];
                VROMatrix4f computedAnimatedBoneTrans = invBind.multiply(computedAnimatedJointTrans[jointI]);
                skeletalFrames[i]->boneIndices.push_back(jointI);
                skeletalFrames[i]->boneTransforms.push_back(computedAnimatedBoneTrans);
            }
        }

        // Remove any KeyFrameAnimations that were "turned into" and used for skeletal animations.
        // If an animation is on a skinner node, it is always treated as a skeletal animation.
        for (auto &jointNode : _skinIndexToJointNodeIndex[skinIndex]) {
            int nodeIndex = jointNode.second;
            _nodeKeyFrameAnims[nodeIndex][skeletalAnimationIndex].clear();
        }

        // Finally construct our skeletal animation
        std::shared_ptr<VROSkeletalAnimation> skeletalAnimation
                = std::make_shared<VROSkeletalAnimation>(currentSkinner, skeletalFrames, totalDuration);
        skeletalAnimation->setName(keyFrameAnim->getName());
        _skinSkeletalAnims[skinIndex].push_back(skeletalAnimation);
    }
    return;
}

void VROGLTFLoader::processSkeletalTransformsForFrame(int skin, int animationIndex, int keyFrameTime,
                                                      int currentJointIndex, std::map<int, VROMatrix4f> &transformsOut){
    // If we are at the root, process its transform to be cascaded down the model's scene tree.
    if (currentJointIndex == 0) {
        int childNodeIndex = _skinIndexToJointNodeIndex[skin][0];
        std::vector<std::shared_ptr<VROKeyframeAnimation>> animations = _nodeKeyFrameAnims[childNodeIndex][animationIndex];
        // TODO support multi-layered keyframe animations
        std::shared_ptr<VROKeyframeAnimation> animation = animations.front();
        const std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> &frames = animation->getFrames();
        VROMatrix4f localTransform;
        localTransform.toIdentity();
        localTransform.scale(frames[keyFrameTime]->scale.x,
                             frames[keyFrameTime]->scale.y,
                             frames[keyFrameTime]->scale.z);
        localTransform = frames[keyFrameTime]->rotation.getMatrix() * localTransform;
        localTransform.translate(frames[keyFrameTime]->translation);
        transformsOut[0] = localTransform;
    }

    // Grab the transform of the current joint to be cascaded and multiplied on the child.
    VROMatrix4f currentMatrix = transformsOut[currentJointIndex];

    // Grab all the child joints for this current joint.
    std::vector<int> childJoints = _skinIndexToJointChildJoints[skin][currentJointIndex];
    for (int childJointIndex : childJoints) {
        // Get the actual node index for the child joint Index and it's animation to set.
        int childNodeIndex = _skinIndexToJointNodeIndex[skin][childJointIndex];

        std::vector<std::shared_ptr<VROKeyframeAnimation>> animations = _nodeKeyFrameAnims[childNodeIndex][animationIndex];
        // TODO support multi-layered keyframe animations
        std::shared_ptr<VROKeyframeAnimation> animation = animations.front();
        
        // Grab the animation transform of the current keyFrame
        const std::vector<std::unique_ptr<VROKeyframeAnimationFrame>> &frames = animation->getFrames();
        VROMatrix4f localTransform;
        localTransform.toIdentity();
        localTransform.scale(frames[keyFrameTime]->scale.x,
                             frames[keyFrameTime]->scale.y,
                             frames[keyFrameTime]->scale.z);
        localTransform = frames[keyFrameTime]->rotation.getMatrix() * localTransform;
        localTransform.translate(frames[keyFrameTime]->translation);

        // Now cascade and compute the actual world computed transform in model space, save it in transformsOut
        VROMatrix4f computedJointTransformInMeshCoords = currentMatrix.multiply(localTransform);
        transformsOut[childJointIndex] = computedJointTransformInMeshCoords;

        // Continue going down the skeletal tree
        processSkeletalTransformsForFrame(skin, animationIndex, keyFrameTime, childJointIndex, transformsOut);
    }

    // We have reached the leaf of the skeletal tree.
    return;
}

void VROGLTFLoader::injectGLTF(std::shared_ptr<VRONode> gltfNode,
                             std::shared_ptr<VRONode> rootNode,
                             std::shared_ptr<VRODriver> driver,
                             std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish) {
    if (gltfNode) {
        // The top-level glTF Node is a dummy; all of the data is stored in the children, so we
        // simply transfer those children over to the destination node
        for (std::shared_ptr<VRONode> child : gltfNode->getChildNodes()) {
            rootNode->addChildNode(child);
        }

        // Recompute the node's umbrellaBoundingBox and set the atomic rendering properties before
        // we notify the user that their glTF has finished loading
        rootNode->recomputeUmbrellaBoundingBox();
        rootNode->syncAppThreadProperties();
        rootNode->setIgnoreEventHandling(rootNode->getIgnoreEventHandling());
        rootNode->setHoldRendering(true);

        // Don't hold a strong reference to the Node: hydrateAsync stores its callback (and
        // thus all copied lambda variables, including the Node) in VROTexture, which can
        // expose us to strong reference cycles (Node <--> Texture). While the callback is
        // cleaned up after hydration completes, it's possible that hydrate will never
        // complete if the Node is quickly removed.
        std::weak_ptr<VRONode> node_w = rootNode;
        VROModelIOUtil::hydrateAsync(rootNode, [node_w, onFinish] {
            std::shared_ptr<VRONode> node_s = node_w.lock();
            if (node_s) {
                if (onFinish) {
                    onFinish(node_s, true);
                }
                node_s->setHoldRendering(false);
            }
        }, driver);
    }
    else {
        if (onFinish) {
            onFinish(rootNode, false);
        }
    }
}

bool VROGLTFLoader::processScene(const tinygltf::Model &gModel, std::shared_ptr<VRONode> rootNode, const tinygltf::Scene &gScene,
                                 std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    sceneNode->setName(gScene.name);

    std::vector<int> gNodeIndexes = gScene.nodes;
    for (int gNodeIndex : gNodeIndexes) {
        // Fail fast if we have failed to process a node in the scene.
        if (!processNode(gModel, sceneNode, gNodeIndex, driver)) {
            return false;
        }
    }

    rootNode->addChildNode(sceneNode);
    return true;
}

bool VROGLTFLoader::processNode(const tinygltf::Model &gModel, std::shared_ptr<VRONode> &parentNode, int gNodeIndex,
                                std::shared_ptr<VRODriver> driver) {
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
    node->setName(gNode.name);

    // Process the Geometry for this node, if any.
    // Fail fast if we have failed to process the node's mesh.
    int meshIndex = gNode.mesh;
    if (meshIndex >= 0 && !processMesh(gModel, node, gModel.meshes[meshIndex], driver)) {
        return false;
    }

    // After processing the nodes of this model, process skins if any.
    std::shared_ptr<VROGeometry> geom = node->getGeometry();
    if (geom != nullptr && gNode.skin >=0) {
        _skinMap[gNode.skin]->setSkinnerNode(node);
        geom->setSkinner(_skinMap[gNode.skin]);
        for (const std::shared_ptr<VROMaterial> &material : geom->getMaterials()) {
            material->addShaderModifier(VROBoneUBO::createSkinningShaderModifier(false));
        }
    }

    // Set the animations on this node, if any.
    if (_nodeKeyFrameAnims.find(gNodeIndex) != _nodeKeyFrameAnims.end()) {
        for (int i = 0; i < _nodeKeyFrameAnims[gNodeIndex].size(); i ++) {
            // Add in parallel all animated keyframe properties within this
            // animation, within this node.
            for (auto anim : _nodeKeyFrameAnims[gNodeIndex][i]) {
                node->addAnimation(anim->getName(), anim);
            }
        }
    }

    if (_skinSkeletalAnims.find(gNode.skin) != _skinSkeletalAnims.end()) {
        for (std::shared_ptr<VROSkeletalAnimation> anim : _skinSkeletalAnims[gNode.skin]){
            node->addAnimation(anim->getName(), anim);
        }
    }

    parentNode->addChildNode(node);

    // Recursively process each child node, if any.
    std::vector<int> gNodeChildrenIndexes = gNode.children;
    for (int gNodeIndex : gNodeChildrenIndexes) {
        // Fail fast if we have failed to process a node in the scene.
        if (!processNode(gModel, node, gNodeIndex, driver)) {
            return false;
        }
    }
    return true;
}

bool VROGLTFLoader::processSkinnerInverseBindData(const tinygltf::Model &gModel,
                                                  const tinygltf::Skin &skin,
                                                  std::vector<VROMatrix4f> &invBindTransformsOut) {
    // Process inverseBind Matrices from the gLTF Accessor
    const tinygltf::Accessor &gDataAcessor = gModel.accessors[skin.inverseBindMatrices];
    GLTFTypeComponent gTypeComponent;
    GLTFType gType;
    if (!getComponentType(gDataAcessor, gTypeComponent) || !getComponent(gDataAcessor, gType)) {
        perr("Invalid Animation channel data provided!");
        return false;
    }

    // Calculate the byte stride size if none is provided from the BufferView.
    const tinygltf::BufferView &gIndiceBufferView = gModel.bufferViews[gDataAcessor.bufferView];
    size_t bufferViewStride = gIndiceBufferView.byteStride;
    if (bufferViewStride == 0) {
        int sizeOfSingleElement = getTypeSize(gType) * getComponentTypeSize(gTypeComponent);
        bufferViewStride = sizeOfSingleElement;
    }

    // Determine offsets and data sizes representing the 'window of data' in the buffer
    size_t elementCount = gDataAcessor.count;
    size_t dataOffset = gDataAcessor.byteOffset + gIndiceBufferView.byteOffset;
    size_t dataLength = elementCount * bufferViewStride;

    // Now process that buffer to produce the right output data.
    const tinygltf::Buffer &gBuffer = gModel.buffers[gIndiceBufferView.buffer];
    std::vector<VROMatrix4f> invBindTransforms;
    VROByteBuffer buffer((char *)gBuffer.data.data() + dataOffset, dataLength, false);
    for (int elementIndex = 0; elementIndex < elementCount; elementIndex++) {

        // Set the buffer position to begin at each element index - Ex: Each Mat4.
        buffer.setPosition((elementIndex * bufferViewStride));

        // For the current element, cycle through each of its float or type component
        float invBindMatrix[16];
        for (int componentCount = 0; componentCount < 16; componentCount ++) {
            if (gTypeComponent == GLTFTypeComponent::Float) {
                float floatData = buffer.readFloat();
                invBindMatrix[componentCount] = floatData;
            } else {
                pwarn("Invalid element type in Animation Inverse Matrix Data: %d", gTypeComponent);
                return false;
            }
        }

        VROMatrix4f mat(invBindMatrix);
        invBindTransforms.push_back(mat);
    }
    invBindTransformsOut = invBindTransforms;
    return true;
}

bool VROGLTFLoader::processMesh(const tinygltf::Model &gModel, std::shared_ptr<VRONode> &rootNode, const tinygltf::Mesh &gMesh,
                                std::shared_ptr<VRODriver> driver) {
    if (gMesh.primitives.size() <=0) {
        perr("GTLF requires mesh data to contain at least one primitive!");
        return false;
    }

    // Prepare our vec of elements (vertex indices) and sources (vertex attributes) to be parsed.
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    std::vector<std::shared_ptr<VROMaterial>> materials;
    std::map<int, std::shared_ptr<VROMorpher>> morphers;

    // A single mesh may contain more than one type of primitive type to be drawn. Cycle through them here.
    const std::vector<tinygltf::Primitive> &gPrimitives = gMesh.primitives;
    for (tinygltf::Primitive gPrimitive : gPrimitives) {

        // Grab vertex indexing information needed for creating meshes.
        bool successVertex = processVertexElement(gModel, gPrimitive, elements);
        bool successAttributes = processVertexAttributes(gModel, gPrimitive.attributes, sources, elements.size() - 1, driver);
        processTangent(elements, sources, elements.size() - 1);
        
        if (!successVertex || !successAttributes) {
            pwarn("Failed to process mesh %s.", gMesh.name.c_str());
            return false;
        }

        // Grab the material pertaining to this primitive in this mesh.
        int gMatIndex = gPrimitive.material;
        std::shared_ptr<VROMaterial> material = nullptr;
        if (gMatIndex >= 0) {
            const tinygltf::Material &gMat = gModel.materials[gMatIndex];
            material = getMaterial(gModel, gMat);
        }

        // Process Morph targets for each primitive.
        if (!processMorphTargets(gModel, gMesh, gPrimitive, material, sources, elements, morphers, driver)) {
            pwarn("Failed to process morph target for mesh %s.", gMesh.name.c_str());
            return false;
        }

        if (material != nullptr) {
            materials.push_back(material);
        }
    }

    // Apply a default material if none has been specified.
    if (materials.size() == 0) {
        materials.push_back(std::make_shared<VROMaterial>());
    }

    // Finally construct our geometry with the processed vertex and attribute data.
    std::shared_ptr<VROGeometry> geometry = std::make_shared<VROGeometry>(sources, elements);
    
    geometry->setName(gMesh.name);
    rootNode->setGeometry(geometry);
    geometry->setMaterials(materials);
    geometry->setMorphers(morphers);
    return true;
}

void VROGLTFLoader::processTangent(std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                                   std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                   size_t geoElementIndex) {
    // Determine if tangent data is missing and thus needs to be calculated.
    std::shared_ptr<VROGeometrySource> pos = nullptr;
    std::shared_ptr<VROGeometrySource> texcoord = nullptr;
    std::shared_ptr<VROGeometrySource> normal = nullptr;
    std::shared_ptr<VROGeometrySource> tangent = nullptr;

    for (auto &source : sources) {
        if (source->getGeometryElementIndex() != geoElementIndex) {
            continue;
        }

        VROGeometrySourceSemantic semantic = source->getSemantic();
        if (semantic == VROGeometrySourceSemantic::Vertex) {
            pos = source;
        } else if (semantic == VROGeometrySourceSemantic::Normal) {
            normal = source;
        } else if (semantic == VROGeometrySourceSemantic::Texcoord) {
            texcoord = source;
        } else if (semantic == VROGeometrySourceSemantic::Tangent) {
            tangent = source;
        }
    }

    // Return if we already have tangent data.
    if (tangent != nullptr) {
        return;
    }

    // Else, to calculate tangent, we MUST have positional, texcoord and normal data.
    if (normal == nullptr || pos == nullptr || texcoord == nullptr) {
        pwarn("Unable to generate missing tangents for this model");
        return;
    }

    std::vector<VROVector3f> posArray;
    pos->processVertices([&posArray](int index, VROVector4f vertex) {
        posArray.push_back(VROVector3f(vertex.x, vertex.y, vertex.z));
    });

    std::vector<VROVector3f> normArray;
    normal->processVertices([&normArray](int index, VROVector4f vertex) {
        normArray.push_back(VROVector3f(vertex.x, vertex.y, vertex.z));
    });

    std::vector<VROVector3f> texCoordArray;
    texcoord->processVertices([&texCoordArray](int index, VROVector4f vertex) {
        texCoordArray.push_back(VROVector3f(vertex.x, vertex.y, vertex.z));
    });

    std::vector<int> elementIndicesArray;
    std::shared_ptr<VROGeometryElement> targetedElement = elements[geoElementIndex];
    targetedElement->processIndices([&elementIndicesArray](int index, int indexRead) {
        elementIndicesArray.push_back(indexRead);
    });

    // Generate the tangents for this model given the attributes for this element index.
    std::vector<VROVector4f> generatedTangents;
    regenerateTangent(posArray, normArray, texCoordArray, elementIndicesArray, generatedTangents);
    if (generatedTangents.size() <= 0) {
        pwarn("Unable to generate tangents for this model");
        return;
    }

    // Finally store the data back into a Tangent geometry source.
    int sizeOfTangent = 4;
    int vertexSize = pos->getVertexCount();
    float *dataOut = new float[vertexSize * sizeOfTangent];
    for (int i = 0; i < vertexSize; i ++ ) {
        dataOut[(i * sizeOfTangent)]     = generatedTangents[i].x;
        dataOut[(i * sizeOfTangent) + 1] = generatedTangents[i].y;
        dataOut[(i * sizeOfTangent) + 2] = generatedTangents[i].z;
        dataOut[(i * sizeOfTangent) + 3] = generatedTangents[i].w;        
    }

    int sizeOfSingleTangent = getTypeSize(GLTFType::Vec4) * getComponentTypeSize(GLTFTypeComponent::Float);
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) dataOut,
                                        vertexSize * sizeOfSingleTangent,
                                        VRODataOwnership::Move);
    tangent = std::make_shared<VROGeometrySource>(indexData,
                                        VROGeometrySourceSemantic::Tangent,
                                        vertexSize,
                                        true,
                                        getTypeSize(GLTFType::Vec4),
                                        getComponentTypeSize(GLTFTypeComponent::Float),
                                        0,
                                        sizeOfSingleTangent);
    tangent->setGeometryElementIndex((int) geoElementIndex);
    sources.push_back(tangent);
    return;
}

void VROGLTFLoader::regenerateTangent(std::vector<VROVector3f> &posArray,
                                      std::vector<VROVector3f> &normArray,
                                      std::vector<VROVector3f> &texCoordArray,
                                      std::vector<int> &elementIndicesArray,
                                      std::vector<VROVector4f> &generatedTangents) {
    int vertexSize = (int) posArray.size();
    // Sanity check.
    if (normArray.size() != vertexSize
        || texCoordArray.size() != vertexSize) {
        pwarn("Unable to missing generate tangents for this model - vertex count does not match.");
        return;
    }

    VROVector3f *tan1 = new VROVector3f[vertexSize * 2];
    VROVector3f *tan2 = tan1 + vertexSize;

    // For each triangle, compute the tangent and bitangent
    for (size_t i = 0; i < elementIndicesArray.size(); i += 3) {
        int i1 = elementIndicesArray[i + 0];
        int i2 = elementIndicesArray[i + 1];
        int i3 = elementIndicesArray[i + 2];

        VROVector3f &var1 = posArray[i1];
        VROVector3f &var2 = posArray[i2];
        VROVector3f &var3 = posArray[i3];

        VROVector3f v1(var1.x, var1.y, var1.z);
        VROVector3f v2(var2.x, var2.y, var2.z);
        VROVector3f v3(var3.x, var3.y, var3.z);

        VROVector3f &texcoord1 = texCoordArray[i1];
        VROVector3f &texcoord2 = texCoordArray[i2];
        VROVector3f &texcoord3 = texCoordArray[i3];

        VROVector3f w1(texcoord1.x, texcoord1.y, 0);
        VROVector3f w2(texcoord2.x, texcoord2.y, 0);
        VROVector3f w3(texcoord3.x, texcoord3.y, 0);

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;

        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;

        float r = 1.0f / (s1 * t2 - s2 * t1);
        VROVector3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        VROVector3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

        // ADD the tangent to the existing value; this way we average across all triangles
        // that share this vertex. Note we do not normalize, and that is intentional: we
        // want larger triangles to have a bigger "impact" than smaller triangles
        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    VROVector3f *tan3 = tan1 + vertexSize;
    for (size_t i = 0; i < vertexSize; i++) {
        VROVector3f n = { normArray[i].x, normArray[i].y, normArray[i].z };
        VROVector3f t = tan1[i];

        // Gram-Schmidt orthogonalize
        VROVector3f tangent = (t - n * n.dot(t)).normalize();

        // Calculate handedness
        float handedness = (n.cross(t).dot(tan3[i]) < 0.0F) ? -1.0F : 1.0F;

        // Ensure that tangent is valid before saving it.
        float magnitude = tangent.magnitude();
        if (!isnan(magnitude) && !isinf(magnitude)) {
            generatedTangents.push_back({tangent.x, tangent.y, tangent.z, handedness});
            continue;
        }

        // Else, generate a default tangent with perpendicular assumptions.
        VROVector3f c1 = n.cross(VROVector3f(0,0,1));
        VROVector3f c2 = n.cross(VROVector3f(0,1,0));
        VROVector3f tangent2;
        if (c1.magnitude() > c2.magnitude()) {
            tangent2 = c1;
        } else {
            tangent2 = c2;
        }

        tangent = tangent.normalize();
        generatedTangents.push_back({tangent2.x, tangent2.y, tangent2.z, -1.0});
    }

    delete[] tan1;
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
    int gPrimitiveIndicesIndex = gPrimitive.indices;
    if (gPrimitiveIndicesIndex < 0) {
        // Fallback to glDrawArrays if no indexed vertices are provided.
        // TODO VIRO-3664: Support Draw Arrays for Viro Geometry in the main render pass.
        pwarn("Models requiring glDrawArray functionality are not yet supported");
        return false;
    }

    // Grab the accessor that maps to a bufferView through which to view the data buffer
    // representing this geometry's indexed vertices.
    const tinygltf::Accessor &gIndicesAccessor = gModel.accessors[gPrimitiveIndicesIndex];
    GLTFTypeComponent gTypeComponent;
    GLTFType gType;
    if (!getComponentType(gIndicesAccessor, gTypeComponent) || !getComponent(gIndicesAccessor, gType)) {
        return false;
    }

    // Warn here if the indexed vertex data is NOT of the proper type expected by GLTF for vertex indexing.
    if (gType != GLTFType::Scalar
        && gTypeComponent != GLTFTypeComponent::UnsignedByte
        && gTypeComponent != GLTFTypeComponent::UnsignedInt
        && gTypeComponent != GLTFTypeComponent::UnsignedShort
        && gTypeComponent != GLTFTypeComponent::Short) {
        perror("Unsupported primitive type provided for GLTF vertex indexes");
        return false;
    }

    // Calculate the byte stride size if none is provided from the BufferView.
    const tinygltf::BufferView &gIndiceBufferView = gModel.bufferViews[gIndicesAccessor.bufferView];
    size_t bufferViewStride = gIndiceBufferView.byteStride;
    if (bufferViewStride == 0) {
        int sizeOfSingleElement = getTypeSize(gType) * getComponentTypeSize(gTypeComponent);
        bufferViewStride = sizeOfSingleElement;
    }

    // Determine offsets and data sizes representing the indexed vertices's 'window of data' in the buffer
    int primitiveCount = VROGeometryUtilGetPrimitiveCount((int) gIndicesAccessor.count, primitiveType);
    size_t elementCount = gIndicesAccessor.count;
    size_t dataOffset = gIndicesAccessor.byteOffset + gIndiceBufferView.byteOffset;
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
                                                   getComponentTypeSize(gTypeComponent),
                                                   gTypeComponent != GLTFTypeComponent::UnsignedByte &&
                                                   gTypeComponent != GLTFTypeComponent::UnsignedShort);
    elements.push_back(element);
    return true;
}

bool VROGLTFLoader::processVertexAttributes(const tinygltf::Model &gModel,
                                            std::map<std::string, int> &gAttributes,
                                            std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                            size_t geoElementIndex,
                                            std::shared_ptr<VRODriver> driver) {
    if (gAttributes.size() <= 0) {
        return false;
    }

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
        GLTFType gType;
        GLTFTypeComponent gTypeComponent;
        if (!getComponentType(gAttributeAccesor, gTypeComponent) || !getComponent(gAttributeAccesor, gType)) {
            return false;
        }
        
        // Determine the offsets and data sizes representing the 'window of data' for this attribute in the buffer
        const tinygltf::BufferView gIndiceBufferView = gModel.bufferViews[gAttributeAccesor.bufferView];
        
        std::shared_ptr<VROGeometrySource> source;

        if (attributeType != VROGeometrySourceSemantic::BoneWeights) {
            size_t bufferViewOffset = gIndiceBufferView.byteOffset;
            size_t bufferViewTotalSize = gIndiceBufferView.byteLength;
            
            // Process and cache the attribute data to be used by the VROGeometrySource associated with
            // this attribute. If we've already been processed (cached) it before, simply grab it.
            std::string key = VROStringUtil::toString(gAttributeAccesor.bufferView);
            std::shared_ptr<VROVertexBuffer> vbo;
            
            auto it = VROGLTFLoader::_dataCache.find(key);
            if (it == VROGLTFLoader::_dataCache.end()) {
                const tinygltf::Buffer &gbuffer = gModel.buffers[gIndiceBufferView.buffer];
                vbo = driver->newVertexBuffer(std::make_shared<VROData>((void *) gbuffer.data.data(), bufferViewTotalSize, bufferViewOffset));
                VROGLTFLoader::_dataCache[key] = vbo;
            } else {
                vbo = it->second;
            }
            source = buildGeometrySource(attributeType, gType, gTypeComponent, gAttributeAccesor, gIndiceBufferView, vbo);
            
        } else {
            const tinygltf::Buffer &gBuffer = gModel.buffers[gIndiceBufferView.buffer];
            source = buildBoneWeightSource(gType, gTypeComponent, gAttributeAccesor, gIndiceBufferView, gBuffer);
        }

        // Because GLTF can have VROGeometryElements that corresponds to different sets of VROGeometrySources,
        // the render needs to be notified of this element-to-source mapping, so that the correct vertex
        // indices are bounded to and pointed by the right set of vertex attributes. Thus, we set the
        // correspondingly mapped VROGeometryElement index here to preserve that mapping.
        source->setGeometryElementIndex((int) geoElementIndex);
        sources.push_back(source);
    }

    return true;
}

bool VROGLTFLoader::processMorphTargets(const tinygltf::Model &gModel,
                                        const tinygltf::Mesh &gMesh,
                                        const tinygltf::Primitive &gPrimitive,
                                        std::shared_ptr<VROMaterial> &mat,
                                        std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                        std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                                        std::map<int, std::shared_ptr<VROMorpher>> &morphers,
                                        std::shared_ptr<VRODriver> driver) {
    // Return early if we have no morph data to process for this geometry.
    if (gPrimitive.targets.size() <= 0) {
        return true;
    }

    // Error out if the mesh does not contain the material required for
    // implementing morph target shaders.
    if (mat == nullptr) {
        return false;
    }

    // Create a base set of geometry sources, required by VROMorpher.
    int elementIndex = (int) (elements.size() - 1);
    std::vector<std::shared_ptr<VROGeometrySource>> baseSources;
    for (std::shared_ptr<VROGeometrySource> &src : sources) {
        if (src->getGeometryElementIndex() == elementIndex) {
            baseSources.push_back(src);
        }
    }

    // Create our VROMorpher class with the base set of geometry sources.
    std::shared_ptr<VROMorpher> morpher = std::make_shared<VROMorpher>(baseSources, mat);

    // Next, grab the morph targets that we'll be applying onto our base geometry in VROMorpher.
    // We parse each raw gLTF morph target below, save it in our VROMorpher, and refer to it by
    // it's name / key.
    for (int targetIndex = 0; targetIndex < gPrimitive.targets.size(); targetIndex ++) {
        std::vector<std::shared_ptr<VROGeometrySource>> morphTargetSources;
        std::map<std::string, int> gAttributes = gPrimitive.targets[targetIndex];
        if (!processVertexAttributes(gModel, gAttributes, morphTargetSources, elements.size() -1, driver)) {
            pwarn("Invalid Attribute found for morph target!");
            return false;
        }

        // Quick sanity check to ensure we have the proper morphing data type.
        for (std::shared_ptr<VROGeometrySource> source : morphTargetSources) {
            if (source->getSemantic() != VROGeometrySourceSemantic::Vertex &&
                source->getSemantic() != VROGeometrySourceSemantic::Tangent &&
                source->getSemantic() != VROGeometrySourceSemantic::Normal) {
                pwarn("Unsupported Attribute found for morph target!");
                return false;
            }
        }

        // Grab the rest of the target's properties.
        std::string key = getMorphTargetName(gModel, gPrimitive, targetIndex);
        float weight = 0.0;
        if (targetIndex < gMesh.weights.size()) {
            weight = gMesh.weights[targetIndex];
        }

        // Add the new target back to the VROMorpher.
        morpher->addTarget(morphTargetSources, key, weight);
    }

    morphers[elementIndex] = morpher;
    return true;
}

std::string VROGLTFLoader::getMorphTargetName(const tinygltf::Model &gModel,
                                              const tinygltf::Primitive &gPrimitive,
                                              int targetIndex) {
    std::string key = VROStringUtil::toString(targetIndex);

    // Iterate through the property accessors of the mesh target to grab it's name.
    std::map<std::string, int> gAttributes = gPrimitive.targets[targetIndex];
    if (gAttributes.size() > 0) {
        for (auto const& gAttribute : gAttributes) {
            int attributeAccessorIndex = gAttribute.second;
            const tinygltf::Accessor &gAttributeAccesor = gModel.accessors[attributeAccessorIndex];
            std::string name = gAttributeAccesor.name;

            // Break out as all accessors will have the same name.
            if (name.size() > 0) {
                key = name;
                break;
            }
        }
    }

    return key;
}

std::shared_ptr<VROGeometrySource> VROGLTFLoader::buildGeometrySource(VROGeometrySourceSemantic attributeType,
                                                                      GLTFType gType,
                                                                      GLTFTypeComponent gTypeComponent,
                                                                      const tinygltf::Accessor &gAttributeAccesor,
                                                                      const tinygltf::BufferView &gIndiceBufferView,
                                                                      std::shared_ptr<VROVertexBuffer> vbo) {
    bool isFloat = gTypeComponent == GLTFTypeComponent::Float;
    size_t attributeAccessorOffset = gAttributeAccesor.byteOffset;
    
    // Calculate the byte stride size if none is provided from the BufferView
    size_t bufferViewStride = gIndiceBufferView.byteStride;
    if (bufferViewStride == 0) {
        int sizeOfSingleElement = getTypeSize(gType) * getComponentTypeSize(gTypeComponent);
        bufferViewStride = sizeOfSingleElement;
    }
    
    // Build the GeometrySource
    int elementCount = (int) gAttributeAccesor.count;
    return std::make_shared<VROGeometrySource>(vbo,
                                               attributeType,
                                               elementCount,
                                               isFloat,
                                               getTypeSize(gType),
                                               getComponentTypeSize(gTypeComponent),
                                               attributeAccessorOffset,
                                               bufferViewStride);
}

std::shared_ptr<VROGeometrySource> VROGLTFLoader::buildBoneWeightSource(GLTFType gType,
                                                                        GLTFTypeComponent gTypeComponent,
                                                                        const tinygltf::Accessor &gAttributeAccesor,
                                                                        const tinygltf::BufferView &gIndiceBufferView,
                                                                        const tinygltf::Buffer &gbuffer) {
    bool isFloat = gTypeComponent == GLTFTypeComponent::Float;
    size_t bufferViewOffset = gIndiceBufferView.byteOffset;
    size_t bufferViewTotalSize = gIndiceBufferView.byteLength;
    
    // Calculate the byte stride size if none is provided from the BufferView.
    size_t bufferViewStride = gIndiceBufferView.byteStride;
    if (bufferViewStride == 0) {
        int sizeOfSingleElement = getTypeSize(gType) * getComponentTypeSize(gTypeComponent);
        bufferViewStride = sizeOfSingleElement;
    }
    
    // gLTF requires the manual normalization of weighted bone attributes. As such,
    // we process them here before constructing our VROGeometrySource.
    VROByteBuffer buffer((char *) gbuffer.data.data() + bufferViewOffset, bufferViewTotalSize, false);
    
    // Parse the gLTF buffers for the weight of each bone and normalize them.
    // The normalized data is stored in dataOut.
    int sizeOfSingleBoneWeight = getTypeSize(gType) * getComponentTypeSize(gTypeComponent);
    float *dataOut = new float[sizeOfSingleBoneWeight * gAttributeAccesor.count]();
    for (int elementIndex = 0; elementIndex < gAttributeAccesor.count; elementIndex++) {
        
        // For the current element, cycle through each of its float or type component
        // and convert them into a float through the math conversions required by gLTF.
        buffer.setPosition(elementIndex * bufferViewStride);
        std::vector<float> weight;
        for (int componentCount = 0; componentCount < getTypeSize(gType); componentCount++) {
            if (gTypeComponent == GLTFTypeComponent::Float) {
                float floatData = buffer.readFloat();
                weight.push_back(floatData);
            } else if (gTypeComponent == GLTFTypeComponent::UnsignedByte) {
                unsigned uByteData = buffer.readUnsignedByte();
                float point = uByteData / 255.0;
                weight.push_back(point);
            } else if (gTypeComponent == GLTFTypeComponent::UnsignedShort) {
                unsigned short uShortData = buffer.readUnsignedShort();
                float point = uShortData / 65535.0;
                weight.push_back(point);
            } else {
                perr("Invalid weighted bone data provided for the 3D glTF skinner.");
                return {};
            }
        }
        
        float totalWeight = weight[0] + weight[1] + weight[2] + weight[3];
        VROVector4f normalizedWeight;
        VROVector4f(weight[0], weight[1], weight[2], weight[3]).scale(1/totalWeight, &normalizedWeight);
        dataOut[(elementIndex * 4)]     = normalizedWeight.x;
        dataOut[(elementIndex * 4) + 1] = normalizedWeight.y;
        dataOut[(elementIndex * 4) + 2] = normalizedWeight.z;
        dataOut[(elementIndex * 4) + 3] = normalizedWeight.w;
    }
    
    // Finally create our geometry sources with the normalized data.
    std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) dataOut,
                                                                   sizeOfSingleBoneWeight * gAttributeAccesor.count,
                                                                   VRODataOwnership::Move);
    return std::make_shared<VROGeometrySource>(indexData,
                                               VROGeometrySourceSemantic::BoneWeights,
                                               gAttributeAccesor.count,
                                               isFloat,
                                               getTypeSize(gType),
                                               getComponentTypeSize(gTypeComponent),
                                               0,
                                               sizeOfSingleBoneWeight);
}

std::shared_ptr<VROMaterial> VROGLTFLoader::getMaterial(const tinygltf::Model &gModel, const tinygltf::Material &gMat) {
    std::shared_ptr<VROMaterial> vroMat = std::make_shared<VROMaterial>();
    tinygltf::ParameterMap gAdditionalMap = gMat.additionalValues;

    // Process PBR values from the given tinyGLTF material into our VROMaterial, if any.
    processPBR(gModel, vroMat, gMat);

    // Process Normal textures
    std::shared_ptr<VROTexture> normalTexture = getTexture(gModel, gAdditionalMap, "normalTexture", false);
    if (normalTexture != nullptr) {
        vroMat->getNormal().setTexture(normalTexture);
    }

    // Process Occlusion Textures
    std::shared_ptr<VROTexture> occlusionTexture = getTexture(gModel, gAdditionalMap, "occlusionTexture", false);
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
    else {
        vroMat->getMetalness().setColor({ (float) metallicFactor, 1.0, 1.0, 1.0 });
        vroMat->getRoughness().setColor({ (float) roughnessFactor, 1.0, 1.0, 1.0 });
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
