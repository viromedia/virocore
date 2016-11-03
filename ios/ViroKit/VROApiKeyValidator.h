//
//  VROApiKeyValidator.h
//  ViroRenderer
//
//  Created by Andy Chu on 10/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void (^VROApiKeyValidatorBlock)(BOOL);

@protocol VROApiKeyValidator <NSObject>

- (void)validateApiKey:(NSString *)apiKey withCompletionBlock:(VROApiKeyValidatorBlock)completionBlock;

@end
