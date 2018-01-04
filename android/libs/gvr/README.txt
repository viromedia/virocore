GVR aidl classes have to be removed for GVR to work alongside ARCore. To do this:

1. Extract sdk-common-X.aar into directory sdk-common-X.aar

2. Use the jarjar library to remove the files (this uses the aidl.txt rules file included in this directory)

java -jar jarjar.jar -rules aidl.txt -output classes.jar sdk-common-X.aar/classes.jar

3. Zip up the AAR files back into an archive. IMPORTANT: do NOT zip the sdk-common-X.aar directory; instead, go *inside* that directory and zip up the files themselves!

