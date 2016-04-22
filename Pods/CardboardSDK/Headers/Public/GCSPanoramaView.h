#import "GCSWidgetView.h"

/** The enum of panorama image types. */
typedef NS_ENUM(int, GCSPanoramaImageType) {
  // Regular image from a single image source, representing pixels for only one eye.
  kGCSPanoramaImageTypeMono = 1,

  // Image in over-under format, from a single image source. Pixels in the upper half correspond to
  // the left eye. Pixels in the lower half correspond to the right eye.
  kGCSPanoramaImageTypeStereoOverUnder,
};

/** Defines a view that can load and display 360-degree panoramic photos. */
@interface GCSPanoramaView : GCSWidgetView

/**
 * Load a 360-Panorama image from |UIImage| of type |kGCSPanoramaImageTypeMono|.
 *
 * If image is nil, it clears the view.
 */
- (void)loadImage:(UIImage *)image;

/**
 * Load a 360-Panorama image from |UIImage| of type |GCSPanoramaImageType|.
 *
 * If image is nil, it clears the view.
 */
- (void)loadImage:(UIImage *)image ofType:(GCSPanoramaImageType)imageType;

@end
