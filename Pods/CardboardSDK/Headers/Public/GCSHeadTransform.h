#import <GLKit/GLKit.h>

/**
 * Enumeration of the left and right eyes, used to identify the correct rendering parameters needed
 * for stereo rendering.
 */
typedef NS_ENUM(NSInteger, GCSEye) {
  kGCSLeftEye,
  kGCSRightEye,
  kGCSCenterEye,
};

/**
 * Defines a struct to hold half field of view angles, in degrees, for an |GCSEye| eye.
 */
typedef struct {
  CGFloat left;
  CGFloat right;
  CGFloat top;
  CGFloat bottom;
} GCSFieldOfView;

/**
 * Defines a class to represent the head transformation for a render frame.
 */
@interface GCSHeadTransform : NSObject

/** Returns the screen viewport for a given eye. */
- (CGRect)viewportForEye:(GCSEye)eye;

/** Returns the projection matrix for the specified eye. */
- (GLKMatrix4)projectionMatrixForEye:(GCSEye)eye near:(CGFloat)near far:(CGFloat)far;

/**
 * Returns the transformation matrix used to convert from Head Space to Eye Space for the given
 * eye.
 */
- (GLKMatrix4)eyeFromHeadMatrix:(GCSEye)eye;

/** Returns the reference matrix of the head pose in start space. */
- (GLKMatrix4)headPoseInStartSpace;

/** Returns the field of view for the specified eye. */
- (GCSFieldOfView)fieldOfViewForEye:(GCSEye)eye;

@end
