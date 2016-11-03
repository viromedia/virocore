//
//  VROApiKey.h
//  ViroRenderer
//
//  Created by Andy Chu on 10/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import <AWSDynamoDB/AWSDynamoDB.h>

@interface VROApiKey : AWSDynamoDBObjectModel <AWSDynamoDBModeling>

@property (nonatomic, strong) NSString *ApiKey;
@property (nonatomic, strong) NSString *Valid;

@end
