//
//  VROLight.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/7/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROLight_h
#define VROLight_h

#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <limits>
#include "VROVector3f.h"
#include "VROVector4f.h"
#include "VROMatrix4f.h"
#include "VROAnimatable.h"

static std::atomic_int sLightId;
class VROTexture;
class VROLightingUBO;

enum class VROLightType {
    Ambient,
    Directional,
    Omni,
    Spot
};

enum class VROShadowMode {
    Forward,
    Deferred,
    Modulated
};

class VROLight : public VROAnimatable {
    
public:
    
    static uint32_t hashLights(const std::vector<std::shared_ptr<VROLight>> &lights);
    
    VROLight(VROLightType type) :
        _lightId(++sLightId),
        _type(type),
        _color({ 1.0, 1.0, 1.0 }),
        _intensity(1000.0),
        _updatedFragmentData(true),
        _updatedVertexData(true),
        _attenuationStartDistance(2.0),
        _attenuationEndDistance(std::numeric_limits<float>::max()),
        _attenuationFalloffExponent(2.0),
        _direction( { 0, 0, -1.0} ),
        _spotInnerAngle(0),
        _spotOuterAngle(45),
        _castsShadow(false),
        _shadowOpacity(1.0),
        _shadowMapSize(1024),
        _shadowBias(0.005),
        _shadowOrthographicScale(10),
        _shadowNearZ(-10),
        _shadowFarZ(20)
    {}
    
    ~VROLight()
    {}
    
    uint32_t getLightId() const {
        return _lightId;
    }
    
    VROLightType getType() const {
        return _type;
    }
    
#pragma mark - Light Properties
    
    void setColor(VROVector3f color);
    VROVector3f getColor() const {
        return _color;
    }
    
    void setIntensity(float intensity);
    float getIntensity() const {
        return _intensity;
    }
    
    void setName(std::string name) {
        this->_name = name;
    }
    std::string getName() const {
        return _name;
    }
    
    void setPosition(VROVector3f position);
    VROVector3f getPosition() const {
        return _position;
    }
    
    void setDirection(VROVector3f direction);
    VROVector3f getDirection() const {
        return _direction;
    }
    
    void setAttenuationStartDistance(float attenuationStartDistance);
    float getAttenuationStartDistance() const {
        return _attenuationStartDistance;
    }
    
    void setAttenuationEndDistance(float attenuationEndDistance);
    float getAttenuationEndDistance() const {
        return _attenuationEndDistance;
    }
    
    void setAttenuationFalloffExponent(float attenuationFalloffExponent);
    float getAttenuationFalloffExponent() const {
        return _attenuationFalloffExponent;
    }
    
    void setSpotInnerAngle(float spotInnerAngle);
    float getSpotInnerAngle() const {
        return _spotInnerAngle;
    }
    
    void setSpotOuterAngle(float spotOuterAngle);
    float getSpotOuterAngle() const {
        return _spotOuterAngle;
    }
    
#pragma mark - Shadow Properties
    
    void setCastsShadow(bool castsShadow);
    bool getCastsShadow() const {
        return _castsShadow;
    }
    
    void setShadowOpacity(float shadowOpacity);
    float getShadowOpacity() const {
        return _shadowOpacity;
    }
    
    void setShadowMapSize(int shadowMapSize) {
        _shadowMapSize = shadowMapSize;
    }
    int getShadowMapSize() const {
        return _shadowMapSize;
    }
    
    void setShadowBias(float shadowBias) {
        _shadowBias = shadowBias;
    }
    float getShadowBias() const {
        return _shadowBias;
    }
    
    void setShadowOrthographicScale(float scale) {
        _shadowOrthographicScale = scale;
    }
    float getShadowOrthographicScale() const {
        return _shadowOrthographicScale;
    }

    void setShadowNearZ(float nearZ) {
        _shadowNearZ = nearZ;
    }
    float getShadowNearZ() const {
        return _shadowNearZ;
    }
    
    void setShadowFarZ(float farZ) {
        _shadowFarZ = farZ;
    }
    float getShadowFarZ() const {
        return _shadowFarZ;
    }
    
#pragma mark - Light Implementation
    
    /*
     Lights hold onto their UBOs so that they can propagate their
     updates to them. Each time a light is updated, it pushes the
     update to all of its parent UBOs.
     */
    void addUBO(std::shared_ptr<VROLightingUBO> ubo) {
        _ubos.push_back(ubo);
    }
    void propagateFragmentUpdates();
    void propagateVertexUpdates();
    
    void setTransformedPosition(VROVector3f position) {
        _transformedPosition = position;
    }
    VROVector3f getTransformedPosition() const {
        return _transformedPosition;
    }
    
#pragma mark - Shadow Implementation

    // VIRO-1185 This will likely be removed in favor of a centralized
    //           sampler2DShadowArray
    std::shared_ptr<VROTexture> getShadowMap() const {
        return _shadowMap;
    }
    void setShadowMap(std::shared_ptr<VROTexture> shadowMap) {
        _updatedVertexData = true;
        _shadowMap = shadowMap;
    }
    
    VROMatrix4f getShadowViewMatrix() const {
        return _shadowViewMatrix;
    }
    VROMatrix4f getShadowProjectionMatrix() const {
        return _shadowProjectionMatrix;
    }
    
    void setShadowViewMatrix(VROMatrix4f shadowViewMatrix) {
        _updatedVertexData = true;
        _shadowViewMatrix = shadowViewMatrix;
    }
    void setShadowProjectionMatrix(VROMatrix4f shadowProjectionMatrix) {
        _updatedVertexData = true;
        _shadowProjectionMatrix = shadowProjectionMatrix;
    }
    
private:
    
    uint32_t _lightId;
    VROLightType _type;
    
    /*
     RGB color of the light.
     */
    VROVector3f _color;
    
    /*
     Luminous flux of the light, ranging from 0 to 1000. Modulates the color of the
     light.
     */
    float _intensity;
    
    std::string _name;
    bool _updatedFragmentData;
    bool _updatedVertexData;
    
    /*
     Omni and Spot parameters.
     */
    VROVector3f _position;
    float _attenuationStartDistance;
    float _attenuationEndDistance;
    float _attenuationFalloffExponent;
    
    /*
     Diffuse parameters.
     */
    VROVector3f _direction;
    
    /*
     Spot parameters.
     */
    float _spotInnerAngle;
    float _spotOuterAngle;
    
    /*
     Internal. The position of the light after all its supernode
     transforms have been applied.
     */
    VROVector3f _transformedPosition;
    
    /*
     Weak refs to all UBOs that use this light. Needed so that when this
     light is updated, we can flag the UBO as a whole as requiring an 
     update.
     */
    std::vector<std::weak_ptr<VROLightingUBO>> _ubos;
    
    /*
     True if this light casts shadows.
     */
    bool _castsShadow;
    
    /*
     The opacity of the shadow. 1.0 implies a pitch black shadow.
     */
    float _shadowOpacity;
    
    /*
     The size of the depth map to use to render shadows. Larger sizes produce
     more detailed shadows at higher cost to rendering performance. Lower sizes
     are faster but result in pixelation at the edges.
     */
    int _shadowMapSize;
    
    /*
     The amount of bias to apply to the Z coordinate when performing the shadow
     depth comparison. This reduces shadow acne, but large biases can increase
     "peter panning".
     */
    float _shadowBias;
    
    /*
     This property only applies to directional lights, where an orthographic
     projection is used to render the shadow map. The orthographic scale determines
     the extent of the screen that's visible to the light when rendering the
     shadow map. A larger value means more of the scene will be shadowed, but
     at lower resolution.
     */
    float _shadowOrthographicScale;
    
    /*
     The near and far clipping planes to use when rendering shadows. Shadows are
     only cast by surfaces within these planes.
     */
    float _shadowNearZ, _shadowFarZ;
    
    /*
     The shadow map rendered during the last cycle for this light.
     */
    std::shared_ptr<VROTexture> _shadowMap;
    
    /*
     The view and projection matrices used to transform any point in world
     space into its corresponding texcoord in the light's shadow depth map.
     */
    VROMatrix4f _shadowViewMatrix;
    VROMatrix4f _shadowProjectionMatrix;
    
};

#endif /* VROLight_h */
