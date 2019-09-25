To generate javadocs:

1. Go to this directory
/ViroRenderer/android/doc

2. Run the javadoc command (replace the classpath with your own path to android.jar)

javadoc -classpath /Users/radvani/Library/Android/sdk/platforms/android-25/android.jar -encoding UTF-8 -d javadoc -doclet com.google.doclava.Doclava -docletpath doclava/ViroDoclava-1.0.6-SNAPSHOT.jar:doclava/jsilver-1.0.0.jar -quiet -public -federate android http://d.android.com -federationapi android android-22.xml -templatedir ./templates_viro ../app/src/main/java/com/viro/core/*.java
