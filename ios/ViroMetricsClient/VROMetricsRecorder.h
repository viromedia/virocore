//
//  Header.h
//  ViroRenderer
//
//  Created by Manish Bodhankar on 4/11/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#import "KeenClient.h"

@interface VROMetricsRecorder : NSObject

@property (nonatomic, strong) NSString *viewType;
@property (nonatomic, strong) NSString *platform;

+ (VROMetricsRecorder *)sharedClientWithViewType:(NSString *)viewType
                                     platform:(NSString *)platform;

-(void)recordEvent:(NSString *)event;
@end

