#import "GCSWidgetView.h"

/**
 * Defines a player view that renders a 360 video using OpenGL.
 */
@interface GCSVideoView : GCSWidgetView

/** Load a local or remote video from a url and start playing. */
- (void)loadFromUrl:(NSURL*)videoUrl;

/** Pause the video. */
- (void)pause;

/** Resume the video. */
- (void)resume;

/** Stop the video. */
- (void)stop;

/** Get the duration of the video. */
- (NSTimeInterval)duration;

/** Seek to the target time position of the video. */
- (void)seekTo:(NSTimeInterval)position;

@end

/** Defines a protocol to notify delegates of change in video playback. */
@protocol GCSVideoViewDelegate <GCSWidgetViewDelegate>

/** Called when position of the video playback head changes. */
- (void)videoView:(GCSVideoView*)videoView didUpdatePosition:(NSTimeInterval)position;

@end
