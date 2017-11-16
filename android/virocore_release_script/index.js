#!/usr/bin/env node

var fs = require('fs');

var filesToRemove = [
"internal/AnimationChain.java",
"internal/AnimationGroup.java",
"internal/ARDeclarativeNode.java",
"internal/ARDeclarativePlane.java",
"internal/Image.java",
"internal/ImageTracker.java",
"internal/ImageTrackerOutput.java",
"internal/LazyMaterial.java",
"internal/OpenCV.java",
"SoundData.java",
]

var methodsToDelete = [
  {fileName:"ARScene.java", methods:["public ARScene(boolean declarative)",
                    "public void addARDeclarativeNode(ARDeclarativeNode node)",
                    "public void updateARDeclarativeNode(ARDeclarativeNode node)",
                    "public void removeARDeclarativeNode(ARDeclarativeNode node)"]},
  {fileName:"Geometry.java", methods:["public void copyAndSetMaterials(List<Material> materials)"]},
  {fileName:"Material.java", methods:["Material(LightingModel lightingModel, int diffuseColor, Texture diffuseTexture, float diffuseIntensity,",
                                      "setDiffuseIntensity", "getDiffuseIntensity", "setFresnelExponent", "getFresnelExponent"]},
  {fileName:"Node.java", methods:["setHierarchicalRendering"]},
  {fileName:"Object3D.java", methods:["loadModel(String modelResource, Type type, AsyncObject3DListener asyncObjListener"]},
  {fileName:"OmniLight.java", methods:["OmniLight(long color, float intensity, float attenuationStartDistance"]},
  {fileName:"ParticleEmitter.java", methods:["ParticleModifierFloatArray", "setOpacityModifierLegacy","setScaleModifierLegacy", "setRotationModifierLegacy","setColorModifierLegacy", "setVelocityModifierLegacy", "setAccelerationModifierLegacy"]},
  {fileName:"PhysicsBody.java", methods:["checkIsValidBodyType","checkIsValidShapeType"]},
  {fileName:"Polyline.java", methods:["Polyline(float[][] points, float width)"]},
  {fileName:"Scene.java", methods:["setEffects(String[] effects)"]},
  {fileName:"Sound.java", methods:["Sound(String path, ViroContext viroContext, PlaybackListener listener)", "Sound(SoundData data, ViroContext viroContext, PlaybackListener listener)"]},
  {fileName:"SoundField.java", methods:["SoundField(String path, ViroContext viroContext, PlaybackListener delegate, boolean local)", "SoundField(SoundData data, ViroContext viroContext"]},
  {fileName:"SpatialSound.java", methods:["SpatialSound(String path, ViroContext viroContext, PlaybackListener delegate", "SpatialSound(SoundData data, ViroContext viroContext"]},
  {fileName:"Sphere.java", methods:["setVideoTexture"]},
  {fileName:"Spotlight.java", methods:["Spotlight(long color, float intensity, float attenuationStartDistance"]},
  {fileName:"Surface.java", methods:["Surface(float width, float height, float u0", "Surface(float width, float height, float u0, float v0", "setVideoTexture", "setImageTexture", "setMaterial", "clearMaterial"]},
  {fileName:"Texture.java", methods:["Texture()", "Texture(Image px, Image nx, Image py,", "Texture(Image image, Format format, boolean sRGB, boolean mipmap)", "Texture(Image image, Format format, boolean sRGB, boolean mipmap, String stereoMode)" ]},
  {fileName:"ViroContext.java", methods:["getCameraOrientation"]},
]

console.log("going to delete files!!");
console.log(__dirname);
var pathIndex = __dirname.lastIndexOf("/");
var path = __dirname.slice(0, pathIndex);
var virocorePath = path + "/app/src/main/java/com/viro/core/"
console.log("virocore path: " + virocorePath);

for(var i =0; i<filesToRemove.length; i++) {
    fs.unlinkSync(virocorePath + filesToRemove[i]);
}


deleteMethods();


function deleteMethods() {
  for(var i =0; i<methodsToDelete.length; i++) {
    var methodToDelete = methodsToDelete[i];
    console.log("reading file: " + virocorePath + methodToDelete.fileName);
    var fileContents = fs.readFileSync(virocorePath + methodToDelete.fileName, "utf-8");
    for(var j = 0; j<methodToDelete.methods.length; j++) {
      var newFileContents = deleteMethod(fileContents, methodToDelete.methods[j]);
      fileContents = newFileContents;
    }

    fs.writeFileSync(virocorePath + methodToDelete.fileName, fileContents);
  }
}

function deleteMethod(fileContents, methodToDelete) {
  console.log("Attempting to delete method:" + methodToDelete);
  var lines = fileContents.split('\n');
  var endLineIndex = -1;
  var startLineIndex = -1;
  for(var i =0; i<lines.length; i++) {
      //console.log(lines[i]);
      var index = lines[i].indexOf(methodToDelete);
      if(index != -1) {
         console.log("Found start line:" + i);
         startLineIndex = i;
         endLineIndex = findEndLine(i, lines);
         break;
      }
  }

  if(endLineIndex != -1 && startLineIndex != -1) {
    var newContents = "";
    for(var i=0; i<lines.length; i++) {
      if(i>=startLineIndex && i<=endLineIndex) {
        continue;
      }
      newContents+=lines[i] + '\n';
    }
    return newContents;
  } else{
      console.log("Unable to find method: " + methodToDelete)
  }
}

function findEndLine(index, lines) {
  for(var i = index; i< lines.length; i++) {
    if(lines[i].indexOf("}") != -1) {
      console.log("Found end line: " + i);
      return i;
    }
  }
  console.log("Unable to find end line!");
}

console.log("Deleted files!");
