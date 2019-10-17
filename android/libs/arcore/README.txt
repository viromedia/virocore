ARCore .so files to be moved into viroar/src/main/jniLibs so that native code can access them. They have to be *removed* from the ARCore AAR file as well, to prevent conflicts. To do this:

1. Extract core-X.Y.Z.aar into directory core-X.Y.Z.aar (unzip core-X.Y.Z.aar -d tempFolder)

2. Remove the C .so files and place them in viroar/src/main/jniLibs/[ARCH]

3. Do NOT remove the JNI .so files: those need to remain in the AAR.

4. Zip up the AAR files back into an archive. (jar cvf core-X.Y.Z.aar -C tempFolder/ .)

5. Also update the arcore_c_api.h in ViroRenderer source (you get this by downloading the SDK -> libraries -> include -> arcore_c_api.h)

Note: You can get future ARCore.aar files here:
- https://dl.google.com/dl/android/maven2/com/google/ar/core/<ARCORE_VERSION>/core-<ARCORE_VERSION>.aar
- eg. https://dl.google.com/dl/android/maven2/com/google/ar/core/1.4.0/core-1.4.0.aar
