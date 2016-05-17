//
//  VROShaderBuilder.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/16/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROShaderBuilder.h"
#include "VROLog.h"

VROShaderBuilder::VROShaderBuilder(std::string mainFile) :
    _mainFile(mainFile) {
    
}

VROShaderBuilder::~VROShaderBuilder() {
    
}

void VROShaderBuilder::prependFunction(std::string functionName) {
    _functionNames.push_back(functionName);
}

std::string VROShaderBuilder::buildShader() {
    
    std::string mainText = readFile(_mainFile);
    
    std::string mainFcn("void main()");
    size_t mainFcnIdx = mainText.find(mainFcn);
    
    passert (mainFcnIdx != std::string::npos);
    
    std::string variables = mainText.substr(0, mainFcnIdx);
    std::string mainCode  = mainText.substr(mainFcnIdx);
    
    std::string fcnCode;
    for (int i = (int)_functionNames.size() - 1; i >= 0; i--) {
        std::string fileName = _functionNames[i];
        fcnCode += readFile(fileName);
        fcnCode += "\n";
    }
    
    return variables + fcnCode + mainCode;
    return fcnCode;
}

std::string VROShaderBuilder::readFile(std::string file) {
    NSString *fileString = [[NSBundle bundleWithIdentifier:@"com.viro.ViroKit"]
                            pathForResource:[NSString stringWithUTF8String:file.c_str()] ofType:nil];
    return std::string([[NSString stringWithContentsOfFile:fileString
                                                  encoding:NSUTF8StringEncoding
                                                     error:nil] UTF8String]);
}