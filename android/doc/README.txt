To generate javadocs:

1. Go to this directory
/ViroRenderer/android/doc

2. Run the javadoc command

javadoc -encoding UTF-8 -d javadoc -doclet com.google.doclava.Doclava -docletpath doclava/doclava.jar:doclava/jsilver-1.0.0.jar -quiet -public -templatedir ./templates_viro ../app/src/main/java/com/viro/renderer/jni/*.java ../app/src/main/java/com/viro/renderer/jni/event/*.java
