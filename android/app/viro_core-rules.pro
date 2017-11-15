-dontobfuscate

# Keep all annotation related attributes that can affect runtime
-keepattributes AnnotationDefault

# Understand the @BridgeOnly annotation.
-keep @interface com.viro.core.internal.annotation.BridgeOnly

-assumenosideeffects class com.viro.core.** {
 @com.viro.core.internal.annotation.BridgeOnly <methods>;
 @com.viro.core.internal.annotation.BridgeOnly <fields>;
 @com.viro.core.internal.annotation.BridgeOnly <init>();
}