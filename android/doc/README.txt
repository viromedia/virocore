To generate javadocs:

1. Go to this directory
/ViroRenderer/android/doc

2. Run the javadoc command

javadoc -encoding UTF-8 -d javadoc -doclet com.google.doclava.Doclava -docletpath doclava/doclava-1.0.6.jar -quiet -public ../app/src/main/java/com/viro/renderer/jni/*.java