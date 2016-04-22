#import <Foundation/Foundation.h>

/** High-level Cardboard Audio Engine. The GCSCardboardAudioEngine allows the user
 * to spatialize sound sources in 3D space, including distance and height cues.
 * The GCSCardboardAudioEngine is capable of playing back spatial sound in two
 *  separate ways:
 *
 * The first method, known as Sound Object rendering, allows the user to create
 * a virtual sound source in 3D space. These sources, while spatialized, are fed
 * with mono audio data.
 * The second method allows the user to play back Ambisonic soundfield
 * recordings. Ambisonic recordings are multi-channel audio files which are
 * spatialized all around the listener in 360 degrees. These can be thought of
 * as recorded or prebaked soundfields. They can be of great use for background
 * effects which sound perfectly spatial. Examples include rain noise, crowd
 * noise or even the sound of the ocean off to one side.
 *
 * *****************************************************************************
 *
 * Construction:
 *
 * - (id)initWithRenderingMode:(renderingMode)rendering_mode;
 *
 * Alternatively, using init without parameters will default to binaural high
 * quality mode.
 *
 * RenderingMode is an enum which specifies a global rendering configuration
 * setting:
 *
 * - kRenderingModeStereoPanning:
 * Stereo panning of all Sound Objects. This disables HRTF-based rendering.
 * - kRenderingModeBinauralLowQuality:
 * This renders Sound Objects over a virtual array of 8 loudspeakers arranged in
 * a cube about the listener’s head. HRTF-based rendering is enabled.
 * - kRenderingModeBinauralHighQuality:
 * This renders Sound Objects over a virtual array of 16 loudspeakers arranged
 * in an approximate equidistribution about the listener’s HRTF-based rendering
 * is enabled.
 *
 * If ARC is not enabled, a call to the dealloc method must be made. See the
 * Example Usage snippet below.
 *
 * *****************************************************************************
 *
 * Audio playback can be started and stopped by calling the following
 * two methods:
 *
 * - (bool)start;
 * - (void)stop;
 *
 * *****************************************************************************
 *
 * Sound Files
 *
 * Both mono sound files for use with Sound Objects and multi-channel Ambisonic
 * sound files can be preloaded with a call to the following method:
 *
 * - (bool)preloadSoundFile:(const NSString*)filename;
 *
 * *****************************************************************************
 *
 * Sound Objects
 *
 * The GCSCardboardAudioEngine allows the user to create virtual Sound Objects
 * which can be placed anywhere in space around the listener. These Sound Objects
 * take as input mono audio data which is then spatialized.
 *
 * Once a suitable audio file is preloaded, it can be played back in 3D space
 * with a call to the following function:
 *
 * - (int)createSoundObject:(const NSString*)filename;
 *
 * Here the filename serves as a handle on the preloaded audio file.
 *
 * This method returns an int handle which can be used to refer to the Sound
 * Object as it is manipulated.
 *
 * Playback of a Sound Object can be initiated with a call to:
 *
 * - (void)startSound:(int)soundId loopingEnabled:(bool)loopingEnabled;
 *
 * The loopingEnabled boolean allows the user to specify whether the sound
 * source should repeat continuously or should be played as a “single shot”.
 *
 * A Sound Object’s position in space or loudness can be altered by calling the
 * following methods:
 *
 * - (void)setSoundObjectPosition:(int)soundObjectId
 *                              x:(float)x
 *                              y:(float)y
 *                              z:(float)z;
 *
 * The three variables x, y, z denote the position in Cartesian world space
 * coordinates at which the Sound Object shall appear.
 *
 * - (void)setSoundVolume:(int)soundId volume:(float)volume;
 *
 * The volume variable allows the user to control the loudness of individual
 * sources. This can be useful when some of your mono audio files are
 * intrinsically louder than others. A value of 1.0f ensures that the mono audio
 * amplitude is not modified.
 *
 * Caution is encouraged when using very loud (e.g. 0dB FS normalized) mono
 * audio data, audio data that has been heavily dynamic range compressed or when
 * using multiple sources. In order to avoid clipping, individual sound object
 * volumes can be reduced by calling setSoundVolume() method.
 *
 * The user can ensure that the Sound Object is currently playing before calling
 * the above methods with a call to:
 *
 * - (bool)isSoundPlaying:(int)soundId;
 *
 * Once one is finished with a sound object and wish to remove it, simply place
 * a call to:
 *
 * - (void)stopSound:(int)soundId;
 *
 * On making a call to this function the Sound Object is destroyed and the
 * corresponding integer handle no longer refers to a valid Sound Object.
 *
 * *****************************************************************************
 *
 * Ambisonic Soundfields
 *
 * The GCSCardboardAudioEngine is also designed to play back Ambisonic
 * soundfields. These are captured or pre rendered 360 degree recordings. It is
 * best to think of them as equivalent to 360 degree video. While they envelop
 * and surround the listener, they only react to rotational movement of the
 * listener. That is, one cannot walk towards features in the soundfield.
 * Soundfields are ideal for accompanying 360 degree video playback, for
 * introducing background and environmental effects such as rain or crowd noise,
 * or even for pre baking 3D audio to reduce rendering costs.
 *
 * A preloaded multi channel Ambisonic sound file can be used to create a
 * soundfield with a call to:
 *
 * - (int)createSoundfield:(const NSString*)filename;
 *
 * Once again an integer handle is returned allowing the user to begin playback
 * of the soundfield, to alter the soundfield’s volume or to stop soundfield
 * playback and as such destroy the object.
 *
 *
 * - (void)startSound:(int)soudnObjectId loopingEnabled:(bool)loopingEnabled;
 * - (void)setSoundVolume:(int)soundId volume:(float)volume;
 * - (void)stopSound:(int)soundId;
 *
 * *****************************************************************************
 *
 * Listener position and Rotation.
 *
 * In order to ensure that the audio in your application reacts to user head
 * movement it is important to update head orientation in the graphics callback
 * using the head orientation matrix.
 *
 * The following two methods control the listener’s head orientation in terms of
 * audio:
 *
 * - (void)setHeadPosition:(float)x y:(float)y z:(float)z;
 *
 * where x, y and z are cartesian world space coordinates
 *
 * and
 *
 * - (void)setHeadRotation:(float)x y:(float)y z:(float)z w:(float)w;
 *
 * Here x, y, z and w are the components of a quaternion.
 *
 * *****************************************************************************
 * // Room effects.
 *
 * The GCSCardboardAudioEngine provides a reverb engine enabling the user to
 * create arbitrary room effects by specifying the size of a room and a material
 * for each surface of the room from the |SurfaceMaterials| enum. Each of these
 * surface materials has unique absorption properties which differ with
 * frequency. The room created will be centred around the listener. Note that in
 * the GCSCardboardAudioEngine the unit of distance is meters.
 *
 * The following methods are used to control room effects:
 *
 * - (void)enableRoom:(bool)enable
 *
 * which enables or disables room effects with smooth transitions.
 *
 * and
 *
 * - (void)setRoomProperties:(float)size_x
 *                    size_y:(float)size_y
 *                    size_z:(float)size_z
 *             wall_material:(MaterialName)wall_material
 *          ceiling_material:(MaterialName)ceiling_material
 *            floor_material:(MaterialName)floor_material;
 *
 * which allows the user to describe the room based on its dimensions (size_x,
 * size_y, size_z), and its surface properties. For example one can expect very
 * large rooms to be more reverberant than smaller rooms and for a room with
 * brick surfaces to be more reverberant than one with heavy curtains on every
 * surface.
 *
 * NB: Sources located outside of a room will sound quite different from those
 * inside due to an attenuation of reverb and direct sound while sources far
 * outside of a room will not be audible.
 *
 * *****************************************************************************
 *
 * Example usage:
 *
 * // Initialize a GCSCardboardAudioEngine in binaural high quality rendering mode.
 * GCSCardboardAudioEngine *cardboardAudio;
 * cardboardAudio = [[GCSCardboardAudioEngine alloc]
 *                   initWithRenderingMode:kRenderingModeBinauralHighQuality];
 *
 * // Load an audio file (compressed or uncompressed) from the main bundle.
 * NSString filename = @"mono_audio_file.mp3";
 * bool filePreloaded = [cardboardAudio preloadSoundFile:filename];
 *
 * // Start audio playback.
 * bool playbackStarted = [cardboardAudio start];
 *
 * // Create a Sound Object with the preloaded audio file.
 * int soundId = -1;
 * if(filePreloaded) {
 *   soundId = [cardboardAudio createSoundObject:filename];
 * }
 *
 * // Begin Playback of the Sound Object.
 * if (soundId != -1) {
 *   [cardboardAudio startSound:soundId loopingEnabled:true];
 * }
 *
 * // Change the location and volume of the Sound Object.
 * if(soundId != -1) {
 *   [cardboardAudio setSoundObjectPosition:soundId x:0.5f y:2.0f z:1.2f];
 *   [cardboardAudio setSoundVolume:0.75f];
 * }
 *
 * // Change the listener position.
 * [cardboardAudio setHeadPosition:0.5f y:0.5f z:0.5f];
 *
 * // Stop playback of the preloaded audio file.
 * if([cardboardAudio isSoundPlaying:soundId]) {
 *   [cardboardAudio stopSound:soundId];
 * }
 *
 * // Stop audio playback.
 * [cardboardAudio stop];
 *
 * // If ARC is not enabled.
 * [cardboardAudio dealloc];
 */
@interface GCSCardboardAudioEngine : NSObject

typedef enum renderingMode {
  kRenderingModeStereoPanning,
  kRenderingModeBinauralLowQuality,
  kRenderingModeBinauralHighQuality,
} renderingMode;

typedef enum materialName {
  kTransparent,
  kAcousticCeilingTiles,
  kBrickBare,
  kBrickPainted,
  kConcreteBlockCoarse,
  kConcreteBlockPainted,
  kCurtainHeavy,
  kFiberGlassInsulation,
  kGlassThin,
  kGlassThick,
  kGrass,
  kLinoleumOnConcrete,
  kMarble,
  kParquetOnConcrete,
  kPlasterRough,
  kPlasterSmooth,
  kPlywoodPanel,
  kPolishedConcreteOrTile,
  kSheetrock,
  kWaterOrIceSurface,
  kWoodCeiling,
  kWoodPanel
} materialName;

/** Initialize with a rendering quality mode. Note, when the default init method
  * is used, the rendering quality is set to kRenderingModeBinauralHighQuality.
  *
  * @param quality Chooses the configuration preset.
  */
- (id)initWithRenderingMode:(renderingMode)rendering_mode;

/** Starts the audio playback.
  *
  *  @return true on success.
  */
- (bool)start;

/** Stops the audio playback. */
- (void)stop;

/** Must be called from the main thread at a regular rate. It is used to execute
 *  background operations outside of the audio thread.
 */
- (void)update;

/** Preloads a local sound file. Note that the local file access method depends
  * on the target platform.
  *
  * @param filename Name of the file used as identifier.
  * @return True on success or if file has been already preloaded.
  */
- (bool)preloadSoundFile:(const NSString*)filename;

/** Returns a new sound object handle. Note that the sample needs to be
  * preloaded and may only contain a single audio channel (mono). The handle
  * automatically destroys itself at the moment the sound playback has stopped.
  *
  * @param filename The path/name of the file to be played.
  * @return Id of new sound object. Returns kInvalidId if the sound file could
  *     not be loaded or if the number of input channels is > 1.
  */
- (int)createSoundObject:(const NSString*)filename;

/** Returns a new ambisonic sound field handle. Note that the sample needs to
  * be preloaded and must have 4 separate audio channels. The handle
  * automatically destroys itself at the moment the sound playback has stopped.
  *
  * @param filename The path/name of the file to be played.
  * @return Id of new soundfield. Returns kInvalidId if the sound file could
  *     not be loaded or if the number of requires input channels does not
  *     match.
  */
- (int)createSoundfield:(const NSString*)filename;

/** Starts the playback of a sound.
  *
  * @param soundId Id of the sound to be played.
  * @param loopingEnabled Enables looped audio playback.
  */
- (void)playSound:(int)soundId loopingEnabled:(bool)loopingEnabled;

/** Stops the playback of a sound and destroys the corresponding Sound Object
  * or Soundfield.
  *
  * @param soundId Id of the sound to be stopped.
  */
- (void)stopSound:(int)soundId;

/** Repositions an existing sound object.
  *
  * @param soundObjectId Id of the sound object to be moved.
  * @param x X coordinate the sound will be placed at.
  * @param y Y coordinate the sound will be placed at.
  * @param z Z coordinate the sound will be placed at.
  */
- (void)setSoundObjectPosition:(int)soundObjectId
                             x:(float)x
                             y:(float)y
                             z:(float)z;

/** Changes the volume of an existing sound.
  *
  * @param soundId Id of the sound to be modified.
  * @param volume Volume value. Should range from 0 (mute) to 1 (max).
  */
- (void)setSoundVolume:(int)soundId volume:(float)volume;

/** Checks if a sound is playing.
  *
  * @param soundId Id of the sound to be checked.
  * @return True if the sound is being played.
  */
- (bool)isSoundPlaying:(int)soundId;

/** Sets the head position.
  *
  * @param x X coordinate of head position in world space.
  * @param y Y coordinate of head position in world space.
  * @param z Z coordinate of head position in world space.
  */
- (void)setHeadPosition:(float)x y:(float)y z:(float)z;

/** Sets the head rotation.
  *
  * @param x X component of quaternion.
  * @param y Y component of quaternion.
  * @param z Z component of quaternion.
  * @param w W component of quaternion.
  */
- (void)setHeadRotation:(float)x y:(float)y z:(float)z w:(float)w;

/** Turns on/off the room reverberation effect.
 *
 *  @param True to enable room effect.
 */
- (void)enableRoom:(bool)enable;

/** Sets the room properties describing the dimensions and surface materials of a given room.
 *
 * @param size_x Dimension along X axis.
 * @param size_y Dimension along Y axis.
 * @param size_z Dimension along Z axis.
 * @param wall_material Surface material for the four walls.
 * @param ceiling_material Surface material for the ceiling.
 * @param floor_material Surface material for the floor.
 */
- (void)setRoomProperties:(float)size_x
                   size_y:(float)size_y
                   size_z:(float)size_z
            wall_material:(materialName)wall_material
         ceiling_material:(materialName)ceiling_material
           floor_material:(materialName)floor_material;

@end
