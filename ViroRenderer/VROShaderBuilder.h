//
//  VROShaderBuilder.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/16/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROShaderBuilder_hpp
#define VROShaderBuilder_hpp

#include <stdio.h>
#include <string>
#include <vector>
#import <Foundation/Foundation.h>

class VROShaderBuilder {
    
public:
    
    /*
     Create a new shader builder. Start with the given file containing
     the main() function.
     */
    VROShaderBuilder(std::string mainFile);
    ~VROShaderBuilder();
    
    /*
     Prepend the function contained in the given file to the shader.
     */
    void prependFunction(std::string functionName);
    
    /*
     Builds the completed shader by appending the various
     functions together, and returns the code.
     */
    std::string buildShader();
    
private:
    
    std::string _mainFile;
    std::vector<std::string> _functionNames;
    
    std::string readFile(std::string file);
    
};

#endif /* VROShaderBuilder_hpp */
