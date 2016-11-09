//
// Created by Raj Advani on 11/9/16.
//

#include "Viro.h"
#include "VROSampleRenderer.h"
#include "VRODriverOpenGLAndroid.h"
#include "VROSceneRendererCardboard.h"

VROSampleRenderer::VROSampleRenderer(std::shared_ptr<VRORenderer> renderer, VROSceneRendererCardboard *sceneRenderer) :
    _sceneRenderer(sceneRenderer) {

    _sceneController = std::make_shared<VROSceneController>(renderer->getReticle(), renderer->getFrameSynchronizer());

    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    //scene->setBackgroundCube([self niagaraTexture]);

    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});

    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.4, 0.4, 0.4 });

    std::shared_ptr<VROLight> spotRed = std::make_shared<VROLight>(VROLightType::Spot);
    spotRed->setColor({ 1.0, 0.0, 0.0 });
    spotRed->setPosition( { -5, 0, 0 });
    spotRed->setDirection( { 1.0, 0, -1.0 });
    spotRed->setAttenuationStartDistance(20);
    spotRed->setAttenuationEndDistance(30);
    spotRed->setSpotInnerAngle(2.5);
    spotRed->setSpotOuterAngle(5.0);

    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.0, 0.0, 1.0 });
    spotBlue->setPosition( { 5, 0, 0 });
    spotBlue->setDirection( { -1.0, 0, -1.0 });
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(2.5);
    spotBlue->setSpotOuterAngle(5.0);

    rootNode->addLight(ambient);
    rootNode->addLight(spotRed);
    rootNode->addLight(spotBlue);

    scene->addNode(rootNode);

    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    box->setName("Box 1");

    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Phong);
/*
    std::vector<std::string> modifierCode =  { "uniform float testA;",
                                               "uniform float testB;",
                                               "_geometry.position.x = _geometry.position.x + testA;"
    };
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                                      modifierCode);

    modifier->setUniformBinder("testA", [](VROUniform *uniform, GLuint location) {
        uniform->setFloat(1.0);
    });
    material->addShaderModifier(modifier);
*/
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});

    rootNode->addChildNode(boxNode);
    //boxNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));

}

VROSampleRenderer::~VROSampleRenderer() {

}

void VROSampleRenderer::setupRendererWithDriver(VRODriver *driver) {
    _driver = driver;
    _sceneRenderer->setSceneController(_sceneController, *_driver);
}
