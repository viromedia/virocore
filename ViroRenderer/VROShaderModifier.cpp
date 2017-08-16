//
//  VROShaderModifier.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROShaderModifier.h"
#include "VROLog.h"
#include "VROUniform.h"
#include "VROAllocationTracker.h"
#include "VROStringUtil.h"
#include <atomic>
#include <algorithm>
#include <sstream>

static std::atomic_int sShaderModifierId;

std::string VROShaderModifier::getShaderModifierKey(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers) {
    std::vector<int> modifierIds;
    for (const std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
        modifierIds.push_back(modifier->getShaderModifierId());
    }
    std::sort(modifierIds.begin(), modifierIds.end());
    
    std::stringstream ss;
    for (int modifierId : modifierIds) {
        ss << "-" << modifierId;
    }
    return ss.str();
}

VROShaderModifier::VROShaderModifier(VROShaderEntryPoint entryPoint, std::vector<std::string> input) :
    _shaderModifierId(++sShaderModifierId),
    _entryPoint(entryPoint) {
    
    for (std::string source : input) {
        if (isVariableDeclaration(source)) {
            _uniforms = _uniforms + source + "\n";
        }
        else {
            _body = _body + source + "\n";
        }
    }
    
    /*
     At the end of each section add the corresponding directive.
     This way multiple shader modifiers can utilize the same entry point.
     */
    _uniforms = _uniforms + getDirective(VROShaderSection::Uniforms) + "\n";
    _body = _body + getDirective(VROShaderSection::Body) + "\n";
        
    ALLOCATION_TRACKER_ADD(ShaderModifiers, 1);
}

VROShaderModifier::~VROShaderModifier() {
    ALLOCATION_TRACKER_SUB(ShaderModifiers, 1);
}

void VROShaderModifier::setUniformBinder(std::string uniform,
                                         VROUniformBindingBlock bindingBlock) {
    
    _uniformBinders[uniform] = bindingBlock;
}

void VROShaderModifier::bindUniform(VROUniform *uniform, GLuint location, const VROGeometry *geometry) {
    auto it = _uniformBinders.find(uniform->getName());
    if (it == _uniformBinders.end()) {
        pabort("No binder was found for uniform %s", uniform->getName().c_str());
    }
    
    VROUniformBindingBlock block = it->second;
    block(uniform, location, geometry);
}

std::vector<std::string> VROShaderModifier::getUniforms() const {
    std::vector<std::string> keys;
    keys.reserve(_uniformBinders.size());
    
    for (auto keyValue : _uniformBinders) {
        keys.push_back(keyValue.first);
    }
    return keys;
}

bool VROShaderModifier::isVariableDeclaration(std::string &line) {
    return VROStringUtil::startsWith(line, "uniform ") ||
           VROStringUtil::startsWith(line, "in ") ||
           VROStringUtil::startsWith(line, "out ");
}

std::string VROShaderModifier::getDirective(VROShaderSection section) const {
    if (_entryPoint == VROShaderEntryPoint::Geometry) {
        if (section == VROShaderSection::Body) {
            return "#pragma geometry_modifier_body";
        }
        else {
            return "#pragma geometry_modifier_uniforms";
        }
    }
    
    else if (_entryPoint == VROShaderEntryPoint::Vertex) {
        if (section == VROShaderSection::Body) {
            return "#pragma vertex_modifier_body";
        }
        else {
            return "#pragma vertex_modifier_uniforms";
        }
    }
    
    else if (_entryPoint == VROShaderEntryPoint::Surface) {
        if (section == VROShaderSection::Body) {
            return "#pragma surface_modifier_body";
        }
        else {
            return "#pragma surface_modifier_uniforms";
        }
    }
    
    else if (_entryPoint == VROShaderEntryPoint::LightingModel) {
        if (section == VROShaderSection::Body) {
            return "#pragma lighting_model_modifier_body";
        }
        else {
            return "#pragma lighting_model_modifier_uniforms";
        }
    }
    
    else if (_entryPoint == VROShaderEntryPoint::Fragment) {
        if (section == VROShaderSection::Body) {
            return "#pragma fragment_modifier_body";
        }
        else {
            return "#pragma fragment_modifier_uniforms";
        }
    }
    
    else if (_entryPoint == VROShaderEntryPoint::Image) {
        if (section == VROShaderSection::Body) {
            return "#pragma image_modifier_body";
        }
        else {
            return "#pragma image_modifier_uniforms";
        }
    }

    // Fill in additional entry points' directives as they are supported
    else {
        pabort();
        return "";
    }
}
