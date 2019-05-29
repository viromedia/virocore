//
//  VROBodyMesheriOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 5/22/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROBodyMesheriOS.h"

#include "VROLog.h"
#include "VROTime.h"
#include "VROMath.h"
#include <mutex>
#include "VRODeviceUtil.h"
#include "VROGeometry.h"
#include "VROPoseFilterMovingAverage.h"
#include "VROPoseFilterLowPass.h"
#include "VROPoseFilterBoneDistance.h"
#include "VROPoseFilterEuro.h"
#include "VROOneEuroFilter.h"
#include "VROMaterial.h"

#define BODYMESH 1

// Set to one of the above
#define VRO_BODY_MESHER_MODEL_A12 BODYMESH
#define VRO_BODY_MESHER_MODEL_A11 BODYMESH

#import "bodymesh.h"

static bool kTestResampling = false;
static const float kConfidenceThreshold = 0.15;
static const float kInitialDampeningPeriodMs = 125;

#pragma mark - Initialization

VROBodyMesheriOS::VROBodyMesheriOS() {
    _isTracking = false;
}

VROBodyMesheriOS::~VROBodyMesheriOS() {
    
}

bool VROBodyMesheriOS::initBodyTracking(VROCameraPosition position,
                                        std::shared_ptr<VRODriver> driver) {
    
    MLModel *model;
    VRODeviceUtil *device = [[VRODeviceUtil alloc] init];
    bool A12 = [device isBionicA12];
    
    if (A12) {
#if VRO_BODY_MESHER_MODEL_A12==BODYMESH
        pinfo("Loading body meshing model");
        model = [[[bodymesh alloc] init] model];
#endif
    } else {
#if VRO_BODY_MESHER_MODEL_A11==BODYMESH
        pinfo("Loading body meshing model");
        model = [[[bodymesh alloc] init] model];
#endif
    }
    
    NSString *uvmapName = @"vbml_m";
    
    _uvTexcoords = loadNumpyArray(uvmapName, @"texcoords");
    _uvMask = loadNumpyArray(uvmapName, @"uv_mask");
    _uvVtoVt = loadNumpyArray(uvmapName, @"v_to_vt");
    _uvFaceToV = loadNumpyArray(uvmapName, @"face_to_v");
    
    _testUv = loadNumpyArray(@"test_uv");
    pinfo("Loaded Test UV array: word size %d, count %d, shape (%d, %d, %d)",
          (int) _testUv.word_size, (int) _testUv.num_vals, (int) _testUv.shape[0], (int) _testUv.shape[1], (int) _testUv.shape[2]);
    
    _visionEngine = std::make_shared<VROVisionEngine>(model, 224, position, VROCropAndScaleOption::CoreML_Fit);
    std::shared_ptr<VROVisionEngineDelegate> delegate = std::dynamic_pointer_cast<VROVisionEngineDelegate>(shared_from_this());
    passert (delegate != nullptr);
    _visionEngine->setDelegate(delegate);
    
    _poseFilter = std::make_shared<VROPoseFilterEuro>(kInitialDampeningPeriodMs, kConfidenceThreshold);
    return true;
}

cnpy::NpyArray VROBodyMesheriOS::loadNumpyArray(NSString *prefix, NSString *array) {
    cnpy::NpyArray arr = loadNumpyArray([NSString stringWithFormat:@"%@_%@", prefix, array]);
    if (arr.shape.size() == 1) {
        pinfo("Loaded array %@: word size %d, count %d, shape (%d)",
              array, (int) arr.word_size, (int) arr.num_vals, (int) arr.shape[0]);
    } else {
        pinfo("Loaded array %@: word size %d, count %d, shape (%d, %d)",
              array, (int) arr.word_size, (int) arr.num_vals, (int) arr.shape[0], (int) arr.shape[1]);
    }
    return arr;
}

cnpy::NpyArray VROBodyMesheriOS::loadNumpyArray(NSString *name) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *texcoordsPath = [bundle pathForResource:name ofType:@"npy"];
    return cnpy::npy_load([texcoordsPath UTF8String]);
}

void VROBodyMesheriOS::setDampeningPeriodMs(double period) {
    _dampeningPeriodMs = period;
    if (period <= 0) {
        _poseFilter = nullptr;
    } else {
        _poseFilter = std::make_shared<VROPoseFilterEuro>(_dampeningPeriodMs, kConfidenceThreshold);
    }
}

double VROBodyMesheriOS::getDampeningPeriodMs() const {
    return _dampeningPeriodMs;
}

void VROBodyMesheriOS::startBodyTracking() {
    _isTracking = true;
}

void VROBodyMesheriOS::stopBodyTracking() {
    _isTracking = false;
}

#pragma mark - Renderer Thread

void VROBodyMesheriOS::update(const VROARFrame *frame) {
    if (!_isTracking) {
        return;
    }
    _visionEngine->update(frame);
}

#pragma mark - Vision Queue (post-processing CoreML output)

// Invoked on the _visionQueue
std::vector<std::pair<VROVector3f, float>> VROBodyMesheriOS::processVisionOutput(VNCoreMLFeatureValueObservation *result, VROCameraPosition cameraPosition,
                                                                                  VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace) {
    
    MLMultiArray *uvmap = result.featureValue.multiArrayValue;
    
    // TODO Make this allocation efficient again
    std::vector<std::pair<VROVector3f, float>> imageSpaceJoints(kNumBodyJoints);
    
    std::vector<float> vertices = deriveVertices(uvmap, cameraPosition, visionToImageSpace, imageToViewportSpace, imageSpaceJoints.data());
    std::shared_ptr<VROGeometrySource> vertexSource = buildMeshVertices(vertices);
    
    if (!_bodyMesh) {
        std::vector<std::shared_ptr<VROGeometrySource>> sources = { vertexSource };
        std::vector<std::shared_ptr<VROGeometryElement>> elements = { buildMeshFaces() };
        _bodyMesh = std::make_shared<VROGeometry>(sources, elements);
        
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        material->getDiffuse().setColor({ 1.0, 0.0, 0.0, 1.0 });
        material->setTransparency(0.5f);
        material->setCullMode(VROCullMode::None);
        
        _bodyMesh->setMaterials({ material });
    }
    else {
        _bodyMesh->setSources({ vertexSource });
    }
    
    std::weak_ptr<VROBodyMesheriOS> tracker_w = std::dynamic_pointer_cast<VROBodyMesheriOS>(shared_from_this());
    
    dispatch_async(dispatch_get_main_queue(), ^{
        std::shared_ptr<VROBodyMesheriOS> tracker = tracker_w.lock();
        
        if (tracker && tracker->_isTracking) {
            std::shared_ptr<VROBodyMesherDelegate> delegate = _bodyMeshDelegate_w.lock();
            if (delegate) {
                delegate->onBodyMeshUpdated(vertices, _bodyMesh);
            }
        }
    });
    
    return imageSpaceJoints;
}

std::vector<float> VROBodyMesheriOS::deriveVertices(MLMultiArray *uvmap, VROCameraPosition cameraPosition,
                                                    VROMatrix4f visionToImageSpace, VROMatrix4f imageToViewportSpace,
                                                    std::pair<VROVector3f, float> *outImageSpaceJoints) {
    if (uvmap.shape.count < 3) {
        return {};
    }
    
    passert (uvmap.dataType == MLMultiArrayDataTypeFloat32);
    float *array = (float *) uvmap.dataPointer;
    int stride_c = (int) uvmap.strides[0].integerValue;
    int stride_h = (int) uvmap.strides[1].integerValue;
    
    double *test_array = _testUv.data<double>();
    
    bool *uv_mask = _uvMask.data<bool>();
    size_t *vt_to_v = _uvVtoVt.data<size_t>();
    size_t *texcoords = _uvTexcoords.data<size_t>();
    
    // The sampling kernel is a sorted (by preference, earlier is better) array
    // of offsets we can add to each texture coordinate to sample around said
    // texture coordinate.
    std::vector<std::vector<int>> sampling_kernel = getSamplingKernel(2);
    
    std::vector<VROVector3f> vertices(_uvTexcoords.shape[0]);
    int num_failed_texcoords = 0;
    
    float normMinZ = -200;
    float normMaxZ = 200;
    float normLengthZ = normMaxZ - normMinZ;
    
    // Iterate through each texcoord, and sample its position in the UV map. This
    // gives us the vertex corresponding to the texcoord.
    for (int i = 0; i < _uvTexcoords.shape[0]; i++) {
        
        // Get texcoord[i], which is a 2D index (x, y) in the range
        // (0, width), (0, height). Add the kernel to produce the
        // sampling block, which contains texcoord itself and all
        // adjacent pixels up to the sampling distance.
        int texcoord[2];
        texcoord[0] = (int) texcoords[i * 2 + 0];
        texcoord[1] = (int) texcoords[i * 2 + 1];
        
        std::vector<std::vector<int>> sampling_coords = sampling_kernel;
        for (std::vector<int> &coord : sampling_coords) {
            coord[0] += texcoord[0];
            coord[1] += texcoord[1];
        }
        
        // Sample using the kernel offests; use the first found.
        bool found_something = false;
        for (std::vector<int> coord : sampling_coords) {
            if (uv_mask[coord[0] * 224 + coord[1]] > 0) {
                float xyz[3];
                for (int k = 0; k < 3; k++) {
                    if (!kTestResampling) {
                        xyz[k] = array[k * stride_c + coord[0] * stride_h + coord[1]];
                    } else {
                        xyz[k] = test_array[coord[0] * (224 * 3) + coord[1] * 3 + k];
                    }
                }
                
                vertices[i] = { xyz[0], xyz[1], xyz[2] * normLengthZ + normMinZ };
                vertices[i] = visionToImageSpace.multiply(vertices[i]);
                vertices[i] = imageToViewportSpace.multiply(vertices[i]);
                
                found_something = true;
                break;
            }
        }
                
        if (!found_something) {
            num_failed_texcoords += 1;
            vertices[i] = { 0, 0, 0 };
        }
    }
                    
    // Now we have the resampled vertices, except they're indexed by texcoord
    // indices. We need to re-index them by vertex indices.
    int numVertices = (int) _uvVtoVt.shape[0];
    std::vector<float> resampledVertices(numVertices * 3);
    
    for (int i = 0; i < numVertices; i++) {
        // Get the indices of the texcoords that correspond to vertex index i.
        std::vector<int> texcoordIndices;
        for (int j = 0; j < 4; j++) {
            if (vt_to_v[i * 4 + j] != ULLONG_MAX) {
                texcoordIndices.push_back((int) vt_to_v[i * 4 + j]);
            } else {
                break;
            }
        }
        
        // Find the resampled vertex that corresponds to the texcoord indices.
        // Typically there is only one, but in case there was more than one
        // take the mean.
        VROVector3f vertex = vertices[texcoordIndices[0]];
        for (int j = 1; j < (int) texcoordIndices.size(); j++) {
            vertex += vertices[texcoordIndices[j]];
        }
        vertex = vertex.scale(1.0 / (float) texcoordIndices.size());
        
        resampledVertices[i * 3 + 0] = vertex.x;
        resampledVertices[i * 3 + 1] = vertex.y;
        resampledVertices[i * 3 + 2] = vertex.z;
    }
    
    return resampledVertices;
}

std::vector<std::vector<int>> VROBodyMesheriOS::getSamplingKernel(int distance) {
    int dimension_size = distance * 2 + 1;
    std::vector<std::vector<int>> kernel(dimension_size * dimension_size);
    
    int p = -1;
    int idx = 0;
    for (int n = 0; n < distance + 1; n++) {
        for (int x = -n; x < n + 1; x++) {
            for (int y = -n; y < n + 1; y++) {
                if (abs(x) > p or abs(y) > p) {
                    kernel[idx] = { x, y };
                    idx += 1;
                }
            }
        }
        p = n;
    }
    return kernel;
}

std::shared_ptr<VROGeometrySource> VROBodyMesheriOS::buildMeshVertices(std::vector<float> &vertices) {
    std::shared_ptr<VROData> varData = std::make_shared<VROData>(vertices.data(), vertices.size() * sizeof(float));
    std::shared_ptr<VROGeometrySource> vertexSource = std::make_shared<VROGeometrySource>(varData,
                                                                                          VROGeometrySourceSemantic::Vertex,
                                                                                          vertices.size() / 3,  // Count
                                                                                          true,                 // Float components
                                                                                          3,                    // Components per vertex
                                                                                          sizeof(float),        // Bytes per component
                                                                                          0,                    // Offset
                                                                                          sizeof(float) * 3);   // Stride
    return vertexSource;
}

std::shared_ptr<VROGeometryElement> VROBodyMesheriOS::buildMeshFaces() {
    int numCorners = (int) _uvFaceToV.shape[0];
    size_t *faceToV = _uvFaceToV.data<size_t>();

    std::vector<int> vertex_faces;
    for (int i = 0; i < numCorners; i++) {
        vertex_faces.push_back((int) faceToV[i]);
    }
    
    std::shared_ptr<VROData> facesData = std::make_shared<VROData>(vertex_faces.data(), vertex_faces.size() * sizeof(int));
    std::shared_ptr<VROGeometryElement> facesSource = std::make_shared<VROGeometryElement>(facesData, VROGeometryPrimitiveType::Triangle,
                                                                                           numCorners / 3, sizeof(int));
    return facesSource;
}

