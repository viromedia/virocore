//
//  HTTPCode.h
//
//  Created by Claire Young on 5/13/15.
//  Copyright (c) 2012 Keen Labs. All rights reserved.
//
//  Most code taken from https://github.com/rafiki270/HTTP-Status-Codes-for-Objective-C
//  under MIT license: http://www.opensource.org/licenses/mit-license.php
//  Status codes taken from: http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
//

#import <Foundation/Foundation.h>


typedef NS_ENUM(NSUInteger, HTTPCode) {
    // Informational
    HTTPCode1XXInformationalUnknown = 1,
    HTTPCode100Continue = 100,
    HTTPCode101SwitchingProtocols = 101,
    HTTPCode102Processing = 102,
    
    // Success
    HTTPCode2XXSuccessUnknown = 2,
    HTTPCode200OK = 200,
    HTTPCode201Created = 201,
    HTTPCode202Accepted = 202,
    HTTPCode203NonAuthoritativeInformation = 203,
    HTTPCode204NoContent = 204,
    HTTPCode205ResetContent = 205,
    HTTPCode206PartialContent = 206,
    HTTPCode207MultiStatus = 207,
    HTTPCode208AlreadyReported = 208,
    HTTPCode226IMUsed = 226,
    
    // Redirection
    HTTPCode3XXSuccessUnknown = 3,
    HTTPCode300MultipleChoices = 300,
    HTTPCode301MovedPermanently = 301,
    HTTPCode302Found = 302,
    HTTPCode303SeeOther = 303,
    HTTPCode304NotModified = 304,
    HTTPCode305UseProxy = 305,
    HTTPCode306SwitchProxy = 306,
    HTTPCode307TemporaryRedirect = 307,
    HTTPCode308PermanentRedirect = 308,
    
    // Client error
    HTTPCode4XXSuccessUnknown = 4,
    HTTPCode400BadRequest = 400,
    HTTPCode401Unauthorised = 401,
    HTTPCode402PaymentRequired = 402,
    HTTPCode403Forbidden = 403,
    HTTPCode404NotFound = 404,
    HTTPCode405MethodNotAllowed = 405,
    HTTPCode406NotAcceptable = 406,
    HTTPCode407ProxyAuthenticationRequired = 407,
    HTTPCode408RequestTimeout = 408,
    HTTPCode409Conflict = 409,
    HTTPCode410Gone = 410,
    HTTPCode411LengthRequired = 411,
    HTTPCode412PreconditionFailed = 412,
    HTTPCode413RequestEntityTooLarge = 413,
    HTTPCode414RequestURITooLong = 414,
    HTTPCode415UnsupportedMediaType = 415,
    HTTPCode416RequestedRangeNotSatisfiable = 416,
    HTTPCode417ExpectationFailed = 417,
    HTTPCode418IamATeapot = 418,
    HTTPCode419AuthenticationTimeout = 419,
    HTTPCode420MethodFailureSpringFramework = 420,
    HTTPCode420EnhanceYourCalmTwitter = 420,
    HTTPCode421MisdirectedRequest = 421,
    HTTPCode422UnprocessableEntity = 422,
    HTTPCode423Locked = 423,
    HTTPCode424FailedDependency = 424,
    HTTPCode426UpgradeRequired = 426,
    HTTPCode428PreconditionRequired = 428,
    HTTPCode429TooManyRequests = 429,
    HTTPCode431RequestHeaderFieldsTooLarge = 431,
    HTTPCode440LoginTimeout = 440,
    HTTPCode444NoResponseNginx = 444,
    HTTPCode449RetryWithMicrosoft = 449,
    HTTPCode450BlockedByWindowsParentalControls = 450,
    HTTPCode451RedirectMicrosoft = 451,
    HTTPCode451UnavailableForLegalReasons = 451,
    HTTPCode494RequestHeaderTooLargeNginx = 494,
    HTTPCode495CertErrorNginx = 495,
    HTTPCode496NoCertNginx = 496,
    HTTPCode497HTTPToHTTPSNginx = 497,
    HTTPCode498TokenExpiredInvalid = 498,
    HTTPCode499ClientClosedRequestNginx = 499,
    HTTPCode499TokenRequiredEsri = 499,
    
    
    // Server error
    HTTPCode5XXSuccessUnknown = 5,
    HTTPCode500InternalServerError = 500,
    HTTPCode501NotImplemented = 501,
    HTTPCode502BadGateway = 502,
    HTTPCode503ServiceUnavailable = 503,
    HTTPCode504GatewayTimeout = 504,
    HTTPCode505HTTPVersionNotSupported = 505,
    HTTPCode506VariantAlsoNegotiates = 506,
    HTTPCode507InsufficientStorage = 507,
    HTTPCode508LoopDetected = 508,
    HTTPCode509BandwidthLimitExceeded = 509,
    HTTPCode510NotExtended = 510,
    HTTPCode511NetworkAuthenticationRequired = 511,
    HTTPCode598NetworkReadTimeoutErrorUnknown = 598,
    HTTPCode599NetworkConnectTimeoutErrorUnknown = 599
};

typedef NS_ENUM(NSUInteger, HTTPCodeType) {
    HTTPCodeUnknownType,
    HTTPCode1XXInformational,
    HTTPCode2XXSuccess,
    HTTPCode3XXRedirect,
    HTTPCode4XXClientError,
    HTTPCode5XXServerError
};

@interface HTTPCodes : NSObject

/**
 *  Return description for a specific HTTP status code
 *
 *  @param code Status code definition
 *
 *  @return Description
 */
+ (NSString *)descriptionForCode:(HTTPCode)code;

/**
 *  Return description for a specific HTTP status code
 *
 *  @param code Status code definition
 *
 *  @return code type (info, success, redirect, client error, server error, unknown)
 */
+ (HTTPCodeType)httpCodeType:(HTTPCode)code;

@end
