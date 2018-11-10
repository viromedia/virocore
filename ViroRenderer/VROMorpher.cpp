//
//  VROMorpher.cpp
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROMorpher.h"
#include "VROLog.h"
#include "VROGeometrySource.h"
#include "VRONode.h"
#include "VROShaderProgram.h"
#include "VROGeometry.h"
#include "VROShaderModifier.h"
#include "VROMaterial.h"
#include "VROUniform.h"
#include "VROAnimationFloat.h"

const int kMaxSupportedAttirbutes = 7;
static std::vector<VROGeometrySourceSemantic> VRO_MORPH_TYPES = {VROGeometrySourceSemantic::Vertex,
                                                                 VROGeometrySourceSemantic::Normal,
                                                                 VROGeometrySourceSemantic::Tangent};

VROMorpher::VROMorpher(std::vector<std::shared_ptr<VROGeometrySource>> &baseSources, std::shared_ptr<VROMaterial> mat) {
    _computeLocation = VROMorpher::ComputeLocation::GPU;
    _material = mat;
    _needsUpdate = true;
    _shaderMod = nullptr;

    passert(baseSources.size() > 0);

    _baseTarget = std::make_shared<VROMorphTarget>();
    for (std::shared_ptr<VROGeometrySource> &src: baseSources) {
        _baseTarget->geometrySources[src->getSemantic()] = src;
    }
    _baseTarget->startWeight = 1.0;
    _baseTarget->endWeight   = 1.0;
    _baseTarget->isCPU = false;
    _geometryElementIndex = baseSources.front()->getGeometryElementIndex();
}

VROMorpher::~VROMorpher() {
    //No-op
}

void VROMorpher::addTarget(std::vector<std::shared_ptr<VROGeometrySource>> morphSources, std::string name, float weight) {
    if (morphSources.size() <= 0) {
        perr("Invalid source Target provided to VROMorpher.");
        return;
    }

    // Fallback to CPU rendering mode if we have more targets than
    // can be handled in GPU mode.
    if (_morphTargets.size() >= kMaxSupportedAttirbutes && _computeLocation == ComputeLocation::GPU) {
        setComputeLocation(ComputeLocation::CPU);
    }

    std::shared_ptr<VROMorphTarget> target = std::make_shared<VROMorphTarget>();
    for (std::shared_ptr<VROGeometrySource> &src: morphSources) {
        target->geometrySources[src->getSemantic()] = src;
    }
    target->startWeight = weight;
    target->endWeight   = weight;
    target->isCPU = false;

    _morphTargets[name] = target;
    configureMorphTargets();
}

void VROMorpher::configureMorphTargets() {
    std::map<std::string, std::shared_ptr<VROMorphTarget>> newTargets;
    if (_computeLocation == VROMorpher::ComputeLocation::CPU) {
        // Ensure our base model morph data is in CPU form.
        _baseTarget = convertMorphTargetToCPU(_baseTarget);

        // Ensure all our morph target data is in CPU form.
        for (auto const& target : _morphTargets) {
            newTargets[target.first] = convertMorphTargetToCPU(target.second);
        }

        // Since all calculations are done on the CPU, no additional
        // shader modifiers or vertex attributes are needed - simply
        // set the new targets.
        _morphTargets = newTargets;
        _needsUpdate = true;
        return;
    }

    if (_computeLocation == VROMorpher::ComputeLocation::Hybrid) {
        // Ensure our base model morph data is in CPU form.
        _baseTarget = convertMorphTargetToCPU(_baseTarget);

        // Ensure all our morph target data is in CPU form.
        for (auto const& target : _morphTargets) {
            newTargets[target.first] = convertMorphTargetToCPU(target.second);
        }
        _morphTargets = newTargets;
        _needsUpdate = true;
        configureShadersHybrid();
        return;
    }

    if (_computeLocation == VROMorpher::ComputeLocation::GPU) {
        // Ensure all our morph target data is in GPU form.
        _baseTarget = convertMorphTargetToGPU(_baseTarget);
        for (auto const& target : _morphTargets) {
            newTargets[target.first] = convertMorphTargetToGPU(target.second);
        }

        _morphTargets = newTargets;
        _needsUpdate = true;
        configureShadersGPU();
        return;
    }

    pabort("Improper render mode for VROMorpher selected");
}

void VROMorpher::configureShadersGPU() {
    // choose the top 4 weighted targets here and process them below.
    int totalAttribs = 0;

    // First we'll need to verify morph attribute weights. To do that, firstly
    // create a vector of weights to be compared against.
    std::vector<std::pair<std::string, float>> gpuKeyWeightPairs;
    for (auto target : _morphTargets) {
        target.second->isActive = false;
        int attribs = target.second->isCPU ?
                           target.second->geometryVecs.size() : target.second->geometrySources.size();
        gpuKeyWeightPairs.push_back(std::make_pair(target.first, target.second->startWeight));
        totalAttribs += attribs;
    }

    // Morph targets must at least have one configure weight.
    if (totalAttribs == 0) {
        perr("No morph targets found when configuring Morpher Shaders!");
        return;
    }

    // If we have more attribute types than slots, greedily choose the top kMaxSupportedAttirbutes.
    if (totalAttribs > kMaxSupportedAttirbutes) {
        // First sort all the morph target by it's weight.
        sort(gpuKeyWeightPairs.begin(), gpuKeyWeightPairs.end(), [=](std::pair<std::string, float>& a,
                                                           std::pair<std::string, float>& b) {
            return a.second < b.second;
        });
    }

    // Now pick the top morph targets until we have kMaxSupportedAttirbutes.
    int includedAttribs = 0;
    std::map<std::string, std::shared_ptr<VROMorphTarget>> chosenGPUTargets;
    for (int i = 0; i < gpuKeyWeightPairs.size(); i ++) {
        std::shared_ptr<VROMorphTarget> &target = _morphTargets[gpuKeyWeightPairs[i].first];

        int attribs = target->isCPU ? target->geometryVecs.size() : target->geometrySources.size();
        if (includedAttribs + attribs <= kMaxSupportedAttirbutes) {
            includedAttribs += attribs;
            target->isActive = true;
            chosenGPUTargets[gpuKeyWeightPairs[i].first] = target;
        }

        // We have already selected the top kMaxSupportedAttirbutes
        if (includedAttribs >= kMaxSupportedAttirbutes) {
            break;
        }
    }

    // Sanity check
    if (chosenGPUTargets.size() > kMaxSupportedAttirbutes) {
        perr("Exceeded maximum number of supported morph attributes.");
        return;
    }

    // Now construct the shader based on those active targets.
    std::vector<std::pair<std::string, VROUniformBindingBlock>> uniformBlocks;
    std::string modVertex = "";
    std::string modNormal = "";
    std::string modTangent = "";
    int morphIndex = 0;
    int attributes = 0;

    for (auto &target : chosenGPUTargets) {
        for (auto semanticTarget : target.second->geometrySources) {
            if (semanticTarget.first == VROGeometrySourceSemantic::Vertex) {
                addMorphModifier(morphIndex, modVertex, false);
            } else if (semanticTarget.first == VROGeometrySourceSemantic::Normal) {
                addMorphModifier(morphIndex, modNormal, false);
            } else if (semanticTarget.first == VROGeometrySourceSemantic::Tangent) {
                addMorphModifier(morphIndex, modTangent, true);
            }


            // Add mod Attribute.
            attributes = attributes | (int) getMorphShaderAttrIndex(morphIndex);

            // Add mod uniform block weight
            std::shared_ptr<VROMorphTarget> morphTarget = target.second;
            VROUniformBindingBlock block = [morphTarget](VROUniform *uniform,
                                                         const VROGeometry *geometry, const VROMaterial *material) {
                uniform->setFloat(morphTarget->startWeight);
            };
            uniformBlocks.push_back(std::make_pair("uniform_" + VROStringUtil::toString(morphIndex), block));

            // Finally set the geometry source.
            semanticTarget.second->setSemantic(getMorphSemanticAttrIndex(morphIndex));
            morphIndex++;
        }
    }

    // Add Morph Target Uniforms based on configured modifiers from active targets.
    std::vector<std::string> modifierCode = {""};
    for (auto block : uniformBlocks) {
        modifierCode.push_back("uniform highp float " + block.first + ";");
    }

    if (modVertex.size() > 0) {
        modVertex = "_geometry.position = _geometry.position " + modVertex + ";";
        modifierCode.push_back(modVertex);
    }

    if (modNormal.size() > 0) {
        modNormal = "_geometry.normal = _geometry.normal " + modNormal + ";";
        modifierCode.push_back(modNormal);

    }

    if (modTangent.size() > 0) {
        modTangent = "_geometry.tangent = _geometry.tangent " + modTangent + ";";
        modifierCode.push_back(modTangent);
    }

    std::shared_ptr<VROShaderModifier> morphMod
            = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry, modifierCode);
    morphMod->setAttributes(attributes);
    for (auto block : uniformBlocks) {
        morphMod->setUniformBinder(block.first, VROShaderProperty::Float, block.second);
    }

    if (_shaderMod != nullptr && morphMod == _shaderMod) {
        return;
    } else if (_shaderMod != nullptr){
        _material->removeShaderModifier(_shaderMod);
    }

    _material->addShaderModifier(morphMod);
    _shaderMod = morphMod;
}

void VROMorpher::configureShadersHybrid() {
    bool hasVertex = false;
    bool hasNormal = false;
    bool hasTangent = false;
    for (auto &target : _morphTargets) {
        for (auto &semanticTarget : target.second->geometryVecs) {
            if (semanticTarget.first == VROGeometrySourceSemantic::Vertex) {
                hasVertex = true;
            } else if (semanticTarget.first == VROGeometrySourceSemantic::Normal) {
                hasNormal = true;
            } else if (semanticTarget.first == VROGeometrySourceSemantic::Tangent) {
                hasTangent = true;
            }
        }
    }

    // For each morph property, iterate through all active morph targets to consolidate
    // and construct the vertex attributes required for the morphing calculations.
    std::vector<std::string> modifierCode = {""};
    int attributes  = 0;

    if (hasVertex) {
        modifierCode.push_back("_geometry.position = mix(_geometry.position, morph_0,  uniform_morph_t);");
        attributes = attributes | (int) getMorphShaderAttrIndex(0);
    } else if (hasNormal) {
        modifierCode.push_back("_geometry.normal   = mix(_geometry.normal,   morph_1,  uniform_morph_t);");
        attributes = attributes | (int) getMorphShaderAttrIndex(1);
    } else if (hasTangent) {
        modifierCode.push_back("_geometry.tangent  = mix(_geometry.tangent,  morph_2,  uniform_morph_t);");
        attributes = attributes | (int) getMorphShaderAttrIndex(2);
    }

    // Add Morph Target Uniforms based on configured modifiers from active targets.
    std::string uniformKey = "uniform_morph_t";
    modifierCode.push_back("uniform highp float uniform_morph_t;");
    std::shared_ptr<VROMorpher> morpher = std::dynamic_pointer_cast<VROMorpher>(shared_from_this());
    VROUniformBindingBlock block = [morpher](VROUniform *uniform,
                                             const VROGeometry *geometry, const VROMaterial *material) {
        if (geometry != nullptr) {
            uniform->setFloat(morpher->_hybridAnimationDuration);
        }
    };

    std::shared_ptr<VROShaderModifier> morphMod = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry, modifierCode);
    morphMod->setAttributes(attributes);
    morphMod->addReplacement("in vec3 morph_2;", "in vec4 morph_2;");
    morphMod->setUniformBinder(uniformKey, VROShaderProperty::Float, block);

    if (_shaderMod != nullptr) {
        _material->removeShaderModifier(_shaderMod);
    }

    _material->addShaderModifier(morphMod);
    _shaderMod = morphMod;
}

bool VROMorpher::update(std::vector<std::shared_ptr<VROGeometrySource>> &geometrySources) {
    if (!_needsUpdate || _morphTargets.size() <= 0) {
        return false;
    }

    /*
     Handle the GPU case, where we load up the additional morph target sources
     into our GPU buffers to handle weight calculations in the Vertex shaders.
     */
    if (_computeLocation == VROMorpher::ComputeLocation::GPU) {
        geometrySources.erase(
                std::remove_if(geometrySources.begin(), geometrySources.end(),
                               [this](const std::shared_ptr<VROGeometrySource>& src) {
                                   return  _geometryElementIndex == src->getGeometryElementIndex() &&
                                           (src->getSemantic() == VROGeometrySourceSemantic::Morph_0
                                          || src->getSemantic() == VROGeometrySourceSemantic::Morph_1
                                          || src->getSemantic() == VROGeometrySourceSemantic::Morph_2
                                          || src->getSemantic() == VROGeometrySourceSemantic::Morph_3
                                          || src->getSemantic() == VROGeometrySourceSemantic::Morph_4
                                          || src->getSemantic() == VROGeometrySourceSemantic::Morph_5
                                          || src->getSemantic() == VROGeometrySourceSemantic::Morph_6);
                               }),
                geometrySources.end());

        for (auto &targetPair : _morphTargets) {
            std::shared_ptr<VROMorphTarget> &target = _morphTargets[targetPair.first];

            // Sanity check.
            if (target->isCPU) {
                pwarn("Attempted to load up cpu target for GPU mode");
                continue;
            }

            // If target is not Active, continue.
            if (!target->isActive) {
                continue;
            }

            for (auto &sources : target->geometrySources) {
                geometrySources.push_back(sources.second);
            }
        }

        _needsUpdate = false;
        return true;
    }

    /*
     Handle the CPU / Hybrid case, where we handle weight calculations on the CPU, after which
     the calculated targets are then loaded in to the GPU vertex shaders.
     */
    std::vector<std::shared_ptr<VROGeometrySource>>::iterator it = geometrySources.begin();
    geometrySources.erase(
            std::remove_if(geometrySources.begin(), geometrySources.end(),
                           [this](const std::shared_ptr<VROGeometrySource>& src) {
                               return _geometryElementIndex == src->getGeometryElementIndex() &&
                                       (src->getSemantic() == VROGeometrySourceSemantic::Vertex
                                      || src->getSemantic() == VROGeometrySourceSemantic::Tangent
                                      || src->getSemantic() == VROGeometrySourceSemantic::Normal
                                      || src->getSemantic() == VROGeometrySourceSemantic::Morph_0
                                      || src->getSemantic() == VROGeometrySourceSemantic::Morph_1
                                      || src->getSemantic() == VROGeometrySourceSemantic::Morph_2);
                           }),
            geometrySources.end());

    if (_computeLocation == VROMorpher::ComputeLocation::CPU) {
        processMorphTargets(true, geometrySources);
    } else if (_computeLocation == VROMorpher::ComputeLocation::Hybrid) {
        processMorphTargets(true, geometrySources);
        processMorphTargets(false, geometrySources);
    }

    _needsUpdate = false;
    return true;
}

void VROMorpher::processMorphTargets(bool isBaseAttribute,
                                     std::vector<std::shared_ptr<VROGeometrySource>> &sourcesOut) {
    std::vector<VROVector4f> &originalMorPosVec = _baseTarget->geometryVecs[VROGeometrySourceSemantic::Vertex];
    std::vector<VROVector4f> &originalMorNormVec = _baseTarget->geometryVecs[VROGeometrySourceSemantic::Normal];
    std::vector<VROVector4f> &originalMorTangentVec = _baseTarget->geometryVecs[VROGeometrySourceSemantic::Tangent];

    // Create the output float data array to be stored in VROGeometry Source.
    float *sourcesPos = originalMorPosVec.size() > 0 ? new float[originalMorPosVec.size() * 3] : nullptr;
    float *sourcesNorm = originalMorNormVec.size() > 0 ? new float[originalMorNormVec.size() * 3] : nullptr;
    float *sourcesTangent = originalMorTangentVec.size() > 0 ? new float[originalMorTangentVec.size() * 4] : nullptr;
    resetSrc3(sourcesPos, originalMorPosVec.size());
    resetSrc3(sourcesNorm, originalMorNormVec.size());
    resetSrc4(sourcesTangent, originalMorTangentVec.size());

    // Create net morph target sources starting with this geometry's original base data.
    addWeightedMorphToSrc3(sourcesPos, originalMorPosVec, 1.0);
    addWeightedMorphToSrc3(sourcesNorm, originalMorNormVec, 1.0);
    addWeightedMorphToSrc4(sourcesTangent, originalMorTangentVec, 1.0);

    // Go through each morph target, add them all up with it's given weight.
    bool hasPos = false;
    bool hasNorm = false;
    bool hasTang = false;
    for (auto &target : _morphTargets) {
        float weight = isBaseAttribute ? target.second->startWeight : target.second->endWeight;
        if (_computeLocation == ComputeLocation::CPU && weight == 0.0) {
            continue;
        }

        std::vector<VROVector4f> &targetMorPosVec = target.second->geometryVecs[VROGeometrySourceSemantic::Vertex];
        std::vector<VROVector4f> &targetMorNormVec = target.second->geometryVecs[VROGeometrySourceSemantic::Normal];
        std::vector<VROVector4f> &targetMorTangentVec = target.second->geometryVecs[VROGeometrySourceSemantic::Tangent];

        if (targetMorPosVec.size() > 0) {
            addWeightedMorphToSrc3(sourcesPos, targetMorPosVec, weight);
            hasPos = true;
        }

        if (targetMorNormVec.size() > 0) {
            addWeightedMorphToSrc3(sourcesNorm, targetMorNormVec, weight);
            hasNorm = true;
        }

        if (targetMorTangentVec.size() > 0) {
            addWeightedMorphToSrc4(sourcesTangent, targetMorTangentVec, weight);
            hasTang = true;
        }

    }

    // Finally set the draw data sources in a VROGeometrySource and add it to sourcesOut.
    if  ((isBaseAttribute && originalMorPosVec.size() > 0) || (!isBaseAttribute && hasPos))  {
        VROGeometrySourceSemantic semantic = isBaseAttribute ? VROGeometrySourceSemantic::Vertex : VROGeometrySourceSemantic::Morph_0;
        std::shared_ptr<VROData> posData = std::make_shared<VROData>((void *) sourcesPos, originalMorPosVec.size() * 3 * sizeof(float));
        std::shared_ptr<VROGeometrySource> posOut = std::make_shared<VROGeometrySource>(posData,
                                                                                        semantic,
                                                                                        originalMorPosVec.size(),
                                                                                        true, 3,
                                                                                        sizeof(float),
                                                                                        0,
                                                                                        sizeof(float) * 3);
        posOut->setGeometryElementIndex(_geometryElementIndex);
        sourcesOut.push_back(posOut);
    }

    if  ((isBaseAttribute && originalMorNormVec.size() > 0) || (!isBaseAttribute && hasNorm))  {
        VROGeometrySourceSemantic semantic = isBaseAttribute ? VROGeometrySourceSemantic::Normal : VROGeometrySourceSemantic::Morph_1;
        std::shared_ptr<VROData> normData = std::make_shared<VROData>((void *) sourcesNorm, originalMorNormVec.size() * 3 * sizeof(float));
        std::shared_ptr<VROGeometrySource> normOut = std::make_shared<VROGeometrySource>(normData,
                                                                                         semantic,
                                                                                         originalMorNormVec.size(),
                                                                                         true, 3,
                                                                                         sizeof(float),
                                                                                         0,
                                                                                         sizeof(float) * 3);
        normOut->setGeometryElementIndex(_geometryElementIndex);
        sourcesOut.push_back(normOut);
    }

    if  ((isBaseAttribute && originalMorTangentVec.size() > 0) || (!isBaseAttribute && hasTang)) {
        VROGeometrySourceSemantic semantic = isBaseAttribute ? VROGeometrySourceSemantic::Tangent : VROGeometrySourceSemantic::Morph_2;
        std::shared_ptr<VROData> tangentData = std::make_shared<VROData>((void *) sourcesTangent, originalMorTangentVec.size() * 4 * sizeof(float));
        std::shared_ptr<VROGeometrySource> tangentOut = std::make_shared<VROGeometrySource>(tangentData,
                                                                                            semantic,
                                                                                            originalMorTangentVec.size(),
                                                                                            true, 4,
                                                                                            sizeof(float),
                                                                                            0,
                                                                                            sizeof(float) * 4);
        tangentOut->setGeometryElementIndex(_geometryElementIndex);
        sourcesOut.push_back(tangentOut);
    }

    if (sourcesPos != nullptr) {
        delete[] sourcesPos;
    }

    if (sourcesNorm != nullptr) {
        delete[] sourcesNorm;
    }

    if (sourcesTangent != nullptr) {
        delete[] sourcesTangent;
    }
}

const std::set<std::string> VROMorpher::getMorphTargetKeys() {
    std::set<std::string> keys;
    for (auto &target : _morphTargets) {
        keys.insert(target.first);
    }
    return keys;
}

void VROMorpher::setWeightForTarget(std::string key, float targetWeight, bool shouldAnimate) {
    if (_morphTargets.find(key) == _morphTargets.end()) {
        pwarn("Attempted to update invalid morph target %s", key.c_str());
        return;
    }

    /*
     If not animating, immediately set the weights on the VROMorphTarget, based
     on the current selected _computeLocation.
     */
    if (!shouldAnimate) {
        if (_computeLocation == VROMorpher::ComputeLocation::CPU) {
            _morphTargets[key]->startWeight = targetWeight;
            _needsUpdate = true;
            return;
        }

        if (_computeLocation == VROMorpher::ComputeLocation::Hybrid) {
            _morphTargets[key]->startWeight = _morphTargets[key]->endWeight;
            _morphTargets[key]->endWeight = targetWeight;
            _hybridAnimationDuration = 1.0;
            _needsUpdate = true;
            return;
        }

        if (_computeLocation == VROMorpher::ComputeLocation::GPU) {
            _morphTargets[key]->startWeight = targetWeight;
            return;
        }
    }

    /*
     Else if animating, update weights in VROMorphTargets through VROAnimations.
     */
    std::shared_ptr<VROMorphTarget> target = _morphTargets[key];
    std::shared_ptr<VROMorpher> morpher = std::dynamic_pointer_cast<VROMorpher>(shared_from_this());

    if (_computeLocation == VROMorpher::ComputeLocation::CPU) {
        animate(std::make_shared<VROAnimationFloat>([target, morpher, targetWeight](VROAnimatable *const animatable, float w) {
                    target->startWeight = w;
                    morpher->_needsUpdate = true;
                }, target->startWeight, targetWeight));
        return;
    }

    if (_computeLocation == VROMorpher::ComputeLocation::GPU) {
        // Switch the weights up, so that we always animate from the previous target weight.
        float currentWeight = target->startWeight;
        target->startWeight = targetWeight;

        // Now animate the weight through VROTransactions
        animate(std::make_shared<VROAnimationFloat>([target, targetWeight](VROAnimatable *const animatable, float w) {
            target->startWeight = w;
        }, currentWeight, targetWeight));
        return;
    }

    if (_computeLocation == VROMorpher::ComputeLocation::Hybrid) {
        // Switch the weights up, so that we always animate from the previous target weight.
        _morphTargets[key]->startWeight = _morphTargets[key]->endWeight;
        _morphTargets[key]->endWeight = targetWeight;
        morpher->_needsUpdate = true;

        // Now animate the weight through VROTransactions, but by performing
        // the interpolation between two targets on the GPU instead via globalMorphT
        animate(std::make_shared<VROAnimationFloat>([target, morpher, targetWeight](VROAnimatable *const animatable, float w) {
            morpher->_hybridAnimationDuration = w;
        }, 0, 1));
        return;
    }
}

const VROMorpher::ComputeLocation VROMorpher::getComputeLocation() {
    return _computeLocation;
}

bool VROMorpher::setComputeLocation(VROMorpher::ComputeLocation mode) {
    if (mode == ComputeLocation::GPU && _morphTargets.size() > kMaxSupportedAttirbutes) {
        pwarn("Unable to set GPU Render Mode for this VROMorpher.");
        return false;
    }

    _computeLocation = mode;
    configureMorphTargets();
    return true;
}

std::shared_ptr<VROMorphTarget> VROMorpher::convertMorphTargetToCPU(std::shared_ptr<VROMorphTarget> targetIn) {
    // Return if target is already of type CPU
    if (targetIn->isCPU) {
        return targetIn;
    }

    if (targetIn->geometrySources.size() <=0) {
        return nullptr;
    }

    std::shared_ptr<VROMorphTarget> targetOut = std::make_shared<VROMorphTarget>();
    for (VROGeometrySourceSemantic type: VRO_MORPH_TYPES) {
        // Skip if there's no source for this type.
        if (targetIn->geometrySources.find(type) == targetIn->geometrySources.end()) {
            continue;
        }

        if (targetIn->geometrySources[type] == nullptr) {
            continue;
        }

        // Else, convert the VROGeometrySource into a vertex array
        std::vector<VROVector4f> dataVec;
        targetIn->geometrySources[type]->processVertices([&dataVec](int i, VROVector4f vertex) {
            dataVec.push_back(vertex);
        });

        if (dataVec.size() > 0) {
            targetOut->geometryVecs[type] = dataVec;
        }
    }

    targetOut->geometrySources.clear();
    targetOut->isCPU = true;
    return targetOut;
}

std::shared_ptr<VROMorphTarget> VROMorpher::convertMorphTargetToGPU(std::shared_ptr<VROMorphTarget> targetIn) {
    // Return if target is already of type GPU.
    if (!targetIn->isCPU) {
        return targetIn;
    }

    if (targetIn->geometryVecs.size() <=0) {
        return nullptr;
    }

    // Grab the new geometry source.
    std::shared_ptr<VROMorphTarget> targetOut = std::make_shared<VROMorphTarget>();
    for (VROGeometrySourceSemantic type: VRO_MORPH_TYPES) {
        // Skip if there's no source for this type.
        if (targetIn->geometryVecs.find(type) == targetIn->geometryVecs.end()) {

            continue;
        }

        if (targetIn->geometryVecs[type].size() <=0) {
            continue;
        }

        std::shared_ptr<VROGeometrySource> source = convertVecToGeoSource(
                targetIn->geometryVecs[type],
                type);

        if (source != nullptr) {
            targetOut->geometrySources[type] = source;
        }
    }

    targetIn->geometryVecs.clear();
    targetOut->isCPU = false;
    return targetOut;
}

std::shared_ptr<VROGeometrySource> VROMorpher::convertVecToGeoSource(
        std::vector<VROVector4f> &dataVecIn, VROGeometrySourceSemantic semantic) {
    if (dataVecIn.size() <=0) {
        perr("Attempted to convert to GeometrySource with invalid data.");
        return nullptr;
    }

    int componentsPerVertex;
    if (semantic == VROGeometrySourceSemantic::Tangent) {
        componentsPerVertex = 4;
    } else if (semantic == VROGeometrySourceSemantic::Vertex) {
        componentsPerVertex = 3;
    } else if (semantic == VROGeometrySourceSemantic::Normal) {
        componentsPerVertex = 3;
    } else {
        perr("Warning, ");
        return nullptr;
    }

    float *sourcesData = (dataVecIn.size() > 0) ? new float[dataVecIn.size() * componentsPerVertex] : nullptr;
    int morphSize = dataVecIn.size();
    for (int i = 0; i < morphSize; i ++) {
        sourcesData[ i * componentsPerVertex]           = dataVecIn[i].x;
        sourcesData[(i * componentsPerVertex) + 1]      = dataVecIn[i].y;
        sourcesData[(i * componentsPerVertex) + 2]      = dataVecIn[i].z;
        if (componentsPerVertex > 3){
            sourcesData[(i * componentsPerVertex) + 3]  = dataVecIn[i].w;
        }
    }
    std::shared_ptr<VROData> data = std::make_shared<VROData>((void *) sourcesData, dataVecIn.size() * componentsPerVertex * sizeof(float));
    std::shared_ptr<VROGeometrySource> geoSource = std::make_shared<VROGeometrySource>(data,
                                                                                       semantic,
                                                                                       dataVecIn.size(),
                                                                                       true, componentsPerVertex,
                                                                                       sizeof(float),
                                                                                       0,
                                                                                       sizeof(float) * componentsPerVertex);
    geoSource->setGeometryElementIndex(_geometryElementIndex);
    return geoSource;
}


VROGeometrySourceSemantic VROMorpher::getMorphSemanticAttrIndex(int i) {
    switch(i) {
        case 0:
            return VROGeometrySourceSemantic::Morph_0;
        case 1:
            return VROGeometrySourceSemantic::Morph_1;
        case 2:
            return VROGeometrySourceSemantic::Morph_2;
        case 3:
            return VROGeometrySourceSemantic::Morph_3;
        case 4:
            return VROGeometrySourceSemantic::Morph_4;
        case 5:
            return VROGeometrySourceSemantic::Morph_5;
        case 6:
            return VROGeometrySourceSemantic::Morph_6;
    }
    pabort("Invalid Geometry Source selected");
}

VROShaderMask VROMorpher::getMorphShaderAttrIndex(int i) {
    switch(i){
        case 0:
            return VROShaderMask::Morph_0;
        case 1:
            return VROShaderMask::Morph_1;
        case 2:
            return VROShaderMask::Morph_2;
        case 3:
            return VROShaderMask::Morph_3;
        case 4:
            return VROShaderMask::Morph_4;
        case 5:
            return VROShaderMask::Morph_5;
        case 6:
            return VROShaderMask::Morph_6;
    }
    pabort("Invalid Geometry Mask selected");
}

void VROMorpher::addMorphModifier(int morphTargetIndex, std::string &code, bool isVec4) {
    std::string targetIndex = VROStringUtil::toString(morphTargetIndex);
    if (isVec4) {
        code = code +  " + vec4((uniform_" + VROStringUtil::toString(morphTargetIndex) + " * " + "morph_" + targetIndex + "), 1.0)";
    } else {
        code = code +  " + (uniform_" + VROStringUtil::toString(morphTargetIndex) + " * " + "morph_" + targetIndex + ")";
    }
}



inline void VROMorpher::addWeightedMorphToSrc3(float *srcDataOut,
                                               std::vector<VROVector4f> &morphData,
                                               float weight) {
    int morphSize = morphData.size();
    for (int i = 0; i < morphSize; i ++) {
        srcDataOut[i *  3]      += (morphData[i].x * (weight));
        srcDataOut[(i * 3) + 1] += (morphData[i].y * (weight));
        srcDataOut[(i * 3) + 2] += (morphData[i].z * (weight));
    }
}

inline void VROMorpher::addWeightedMorphToSrc4(float *srcDataOut,
                                               std::vector<VROVector4f> &morphData,
                                               float weight) {
    int morphSize = morphData.size();
    for (int i = 0; i < morphSize; i ++) {
        srcDataOut[i *  4]       += (morphData[i].x * (weight));
        srcDataOut[(i * 4) + 1] += (morphData[i].y * (weight));
        srcDataOut[(i * 4) + 2] += (morphData[i].z * (weight));
        srcDataOut[(i * 4) + 3] += (morphData[i].z * (weight));
    }
}

inline void VROMorpher::resetSrc3(float *srcDataOut, int size) {
    for (int i = 0; i < size; i ++) {
        srcDataOut[i *  3]      = 0;
        srcDataOut[(i * 3) + 1] = 0;
        srcDataOut[(i * 3) + 2] = 0;
    }
}

inline void VROMorpher::resetSrc4(float *srcDataOut, int size) {
    for (int i = 0; i < size; i ++) {
        srcDataOut[i *  4]      = 0;
        srcDataOut[(i * 4) + 1] = 0;
        srcDataOut[(i * 4) + 2] = 0;
        srcDataOut[(i * 4) + 3] = 0;
    }
}
