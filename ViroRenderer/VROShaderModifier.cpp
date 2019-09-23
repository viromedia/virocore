//
//  VROShaderModifier.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
    _attributes(0),
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
    _body = _body + "\n" + getDirective(VROShaderSection::Body) + "\n";
        
    ALLOCATION_TRACKER_ADD(ShaderModifiers, 1);
}

VROShaderModifier::~VROShaderModifier() {
    for (auto kv : _uniformBinders) {
        delete (kv.second);
    }
    ALLOCATION_TRACKER_SUB(ShaderModifiers, 1);
}

void VROShaderModifier::setUniformBinder(std::string uniform,
                                         VROShaderProperty type,
                                         VROUniformBindingBlock bindingBlock) {
    VROUniformBinder *binder = new VROUniformBinder(uniform, type, bindingBlock);
    _uniformBinders[uniform] = binder;
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
           VROStringUtil::startsWith(line, "out ") ||
           VROStringUtil::startsWith(line, "layout ") ||
           VROStringUtil::startsWith(line, "#include");
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
