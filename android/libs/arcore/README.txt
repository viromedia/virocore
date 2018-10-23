ARCore .so files to be moved into viroar/src/main/jniLibs so that native code can access them. They have to be *removed* from the ARCore AAR file as well, to prevent conflicts. To do this:

1. Extract core-X.Y.Z.aar into directory core-X.Y.Z.aar

2. Remove the C .so files and place them in viroar/src/main/jniLibs/[ARCH]

3. Do NOT remove the JNI .so files: those need to remain in the AAR.

4. Zip up the AAR files back into an archive. IMPORTANT: do NOT zip the core-X.Y.Z.aar directory; instead, go *inside* that directory and zip up the files themselves! This is to ensure that the resulting AAR doesn't extract out into a directory (the files should be "flat"). Tar it by running this commend from the directory in terminal: 'tar czf core-1.5.0.aar *.*' You untar to test result with 'tar xvzf core-1.5.0.aar'

5. Also update the arcore_c_api.h in ViroRenderer source to match what's in the new aar file.
