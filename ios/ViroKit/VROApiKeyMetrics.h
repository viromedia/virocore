//
//  VROApiKeyMetrics.h
//  ViroRenderer
//
//  Created by Andy Chu on 10/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <AWSDynamoDB/AWSDynamoDB.h>

static NSString *const kVROApiKeyMetricsFormat = @"%@_%@_%@_%@_%@";

@interface VROApiKeyMetrics : AWSDynamoDBObjectModel <AWSDynamoDBModeling>

@property (nonatomic, strong) NSString *ApiKey_BundleId_BuildType;
@property (nonatomic, strong) NSString *Date;
@property (nonatomic, strong) NSNumber *Count;

+ (NSString *)CountAttribute;

- (id)initWithApiKey:(NSString *)apiKey;

@end
