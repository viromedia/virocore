//
//  VROApiMetricsIncrementRequest.h
//  ViroRenderer
//
//  Created by Andy Chu on 10/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VROApiKeyMetrics.h"

/**
 This class encapsulates the request logic for incrementing the Count of the given metricsRecord
 item in the metrics DynamoDB table or adding the item if it doesn't exist.
 */
@interface VROApiMetricsIncrementRequest : NSObject

- (id)initWithMetrics:(VROApiKeyMetrics *)metricsRecord
      dynamoObjMapper:(AWSDynamoDBObjectMapper *)objMapper;

/**
 This function executes this request, it can only ever be called once.
 */
- (void)run;

@end
