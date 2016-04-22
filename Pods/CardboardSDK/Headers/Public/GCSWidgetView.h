#import <UIKit/UIKit.h>

@protocol GCSWidgetViewDelegate;

/** The enum of various widget display modes. */
typedef NS_ENUM(NSInteger, GCSWidgetDisplayMode) {
  // Widget is displayed embedded in other views.
  kGCSWidgetDisplayModeEmbedded = 1,
  // Widget is displayed in fullscreen mono mode.
  kGCSWidgetDisplayModeFullscreen,
  // Widget is displayed in fullscreen VR (stereo) mode.
  kGCSWidgetDisplayModeFullscreenVR,
};

/** Defines a base class for all widget views, that encapsulates common functionality. */
@interface GCSWidgetView : UIView

/** The delegate that is called when the widget view is loaded. */
@property(nonatomic, weak) id<GCSWidgetViewDelegate> delegate;

/** Displays a button that allows the user to transition to fullscreen mode. */
@property(nonatomic) BOOL enableFullscreenButton;

/** Displays a button that allows the user to transition to fullscreen VR mode. */
@property(nonatomic) BOOL enableCardboardButton;

@end

/** Defines a delegate for GCSWidgetView and its subclasses. */
@protocol GCSWidgetViewDelegate<NSObject>

@optional

/**
 * Called when the user taps the widget view. This corresponds to the Cardboard viewer's trigger
 * event.
 */
- (void)widgetViewDidTap:(GCSWidgetView *)widgetView;

/** Called when the widget view's display mode changes. See |GCSWidgetDisplayMode|. */
- (void)widgetView:(GCSWidgetView *)widgetView
    didChangeDisplayMode:(GCSWidgetDisplayMode)displayMode;

/** Called when the content is successfully loaded. */
- (void)widgetView:(GCSWidgetView *)widgetView didLoadContent:(id)content;

/** Called when there is an error loading content in the widget view. */
- (void)widgetView:(GCSWidgetView *)widgetView
    didFailToLoadContent:(id)content
        withErrorMessage:(NSString *)errorMessage;

@end
