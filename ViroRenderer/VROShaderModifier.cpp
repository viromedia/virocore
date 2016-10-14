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

static std::atomic_int sShaderModifierId;

uint32_t VROShaderModifier::hashShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers) {
    uint32_t h = 0;
    for (const std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
        h = 31 * h + modifier->getShaderModifierId();
    }
    return h;
}

VROShaderModifier::VROShaderModifier(VROShaderEntryPoint entryPoint, std::vector<std::string> input) :
    _shaderModifierId(++sShaderModifierId),
    _entryPoint(entryPoint) {
    
    for (std::string source : input) {
        _body = _body + source + "\n";
    }
    _uniforms = extractUniforms(&_body);
}

VROShaderModifier::~VROShaderModifier() {
    
}

void VROShaderModifier::setUniformBinder(std::string uniform,
                                         VROUniformBindingBlock bindingBlock) {
    
    _uniformBinders[uniform] = bindingBlock;
}

void VROShaderModifier::bindUniform(VROUniform *uniform, GLuint location) {
    auto it = _uniformBinders.find(uniform->getName());
    if (it == _uniformBinders.end()) {
        pabort("No binder was found for uniform %s", uniform->getName().c_str());
    }
    
    VROUniformBindingBlock block = it->second;
    block(uniform, location);
}

std::vector<std::string> VROShaderModifier::getUniforms() const {
    std::vector<std::string> keys;
    keys.reserve(_uniformBinders.size());
    
    for (auto keyValue : _uniformBinders) {
        keys.push_back(keyValue.first);
    }
    return keys;
}

std::string VROShaderModifier::extractUniforms(std::string *source) {
    std::string uniforms;
    extractNextUniform(&uniforms, source);
    
    return uniforms;
}

void VROShaderModifier::extractNextUniform(std::string *uniforms, std::string *body) {
    size_t start = body->find("uniform");
    if (start == std::string::npos) {
        return;
    }
    
    size_t end = body->find("\n", start);
    std::string uniform = body->substr(start, end - start + 1);

    uniforms->append(uniform);
    body->replace(start, end - start, "");
    
    return extractNextUniform(uniforms, body);
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

    // Fill in additional entry points directives as they are supported
    else {
        pabort();
        return "";
    }
}
