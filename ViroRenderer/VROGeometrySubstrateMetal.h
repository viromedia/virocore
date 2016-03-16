//
//  VROGeometrySubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROGeometrySubstrateMetal_h
#define VROGeometrySubstrateMetal_h

#include "VROGeometrySubstrate.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include <vector>
#include <memory>
#include <map>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

class VROGeometry;
class VROMaterial;
class VROGeometrySource;
class VROGeometryElement;
class VRORenderContextMetal;
class VROMaterialSubstrateMetal;
class VROConcurrentBuffer;

struct VROVertexArrayMetal {
    id <MTLBuffer> buffer;
};

struct VROGeometryElementMetal {
    id <MTLBuffer> buffer;
    MTLPrimitiveType primitiveType;
    int indexCount;
    MTLIndexType indexType;
    int indexBufferOffset;
};

/*
 Metal representation of a VROGeometry. 
 
 Each set of VROGeometrySources that share the same underlying data buffer are combined
 together into a single VROGeometrySourceMetal, which contains a single MTLBuffer,
 and single MTLVertexDescriptor that describes the interleaved data between the geometry
 sources. At rendering time, all of these vertex buffers are attached to the render encoder
 in successive indexes, starting at 0, via [renderEncoder setVertexBuffer:offset:atIndex:].
 
 For each VROGeometryElement, we create a VROGeometryElementMetal, which contains all the
 information necessary for [renderEncoder drawIndexedPrimitives] using the attached 
 vertex buffers.
 */
class VROGeometrySubstrateMetal : public VROGeometrySubstrate {
    
public:
    
    VROGeometrySubstrateMetal(const VROGeometry &geometry,
                              const VRORenderContextMetal &context);
    virtual ~VROGeometrySubstrateMetal();
    
    void render(const std::vector<std::shared_ptr<VROMaterial>> &materials,
                const VRORenderContext &context,
                VRORenderParameters &params);
    
private:
    
    MTLVertexDescriptor *_vertexDescriptor;
    std::vector<VROVertexArrayMetal> _vars;
    std::vector<VROGeometryElementMetal> _elements;
    
    /*
     Pipeline and depth states for each geometry element. Note that pipeline 
     state is determined by both the geometry (by way of the _vertexDescriptor) 
     and the material; this is why it's not a member of the VROMaterialSubstrate.
     */
    std::vector<id <MTLRenderPipelineState>> _elementPipelineStates;
    std::vector<id <MTLDepthStencilState>> _elementDepthStates;

    /*
     Index of each element to its outgoing material, if it has one. Otherwise
     set to null for that index.
     */
    std::vector<id <MTLRenderPipelineState>> _outgoingPipelineStates;
    
    /*
     Uniforms for the view.
     */
    VROConcurrentBuffer *_viewUniformsBuffer;
    
    /*
     Parse the given geometry elements and populate the _elements vector with the
     results.
     */
    void readGeometryElements(id <MTLDevice> device,
                              const std::vector<std::shared_ptr<VROGeometryElement>> &elements);
    
    /*
     Parse the given geometry sources and populate the _sources vector with the
     results.
     */
    void readGeometrySources(id <MTLDevice> device,
                             const std::vector<std::shared_ptr<VROGeometrySource>> &sources);
    
    /*
     Update the render pipeline state and depth-stencil pipeline state in response to materials
     changing.
     */
    void updatePipelineStates(const VROGeometry &geometry,
                              const VRORenderContextMetal &context);
    
    /*
     Create a pipeline state from the given material, using the current _vertexDescriptor.
     */
    id <MTLRenderPipelineState> createRenderPipelineState(const std::shared_ptr<VROMaterial> &material,
                                                          const VRORenderContextMetal &context);
    
    /*
     Create a depth/stencil state from the given material.
     */
    id <MTLDepthStencilState> createDepthStencilState(const std::shared_ptr<VROMaterial> &material,
                                                      id <MTLDevice> device);
    
    /*
     Parse an MTLVertexFormat from the given geometry source.
     */
    MTLVertexFormat parseVertexFormat(std::shared_ptr<VROGeometrySource> &source);
    
    /*
     Parse an MTLPrimitiveType from the given geometry VROGeometryPrimitiveType.
     */
    MTLPrimitiveType parsePrimitiveType(VROGeometryPrimitiveType primitive);
    
    /*
     Get how many indices are required to render the given number of primitives of the
     given type.
     */
    int getIndicesCount(int primitiveCount, VROGeometryPrimitiveType primitiveType);

    /*
     Parse the attribute index for the given semantic.
     */
    int parseAttributeIndex(VROGeometrySourceSemantic semantic);
    
    /*
     Rendering helper function.
     */
    void renderMaterial(VROMaterialSubstrateMetal *material,
                        VROGeometryElementMetal &element,
                        id <MTLRenderPipelineState> pipelineState,
                        id <MTLDepthStencilState> depthStencilState,
                        id <MTLRenderCommandEncoder> renderEncoder,
                        VRORenderParameters &params,
                        const VRORenderContext &context);
    
};

#endif /* VROGeometrySubstrateMetal_h */
