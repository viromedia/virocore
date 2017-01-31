//
//  VROInputPresenterDaydream.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROInputPresenterDaydream_H
#define VROInputPresenterDaydream_H

#include <memory>
#include <string>
#include <vector>
#include <VROReticle.h>
#include <VROBox.h>
#include <VROOBJLoader.h>
#include <VROPlatformUtil.h>
#include "VRONode.h"
#include "VRORenderContext.h"
#include "VROInputPresenter.h"
#include "VROInputType.h"
#include "VROMaterial.h"

class VROInputPresenterDaydream : public VROInputPresenter {
public:
    VROInputPresenterDaydream(std::shared_ptr<VRORenderContext> context):VROInputPresenter(context) {
        // Initial values required for arm model
        _forwardVector = VROVector3f(0,0,-1);
        attachElbowNode();
        attachControllerNode();
        attachLaserToController();

        std::shared_ptr<VROTexture> reticleTexture
                = std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("ddcursor_pointer.jpg"));
        std::shared_ptr<VROReticle> reticle = std::make_shared<VROReticle>(reticleTexture);
        reticle->setPointerMode(true);
        setReticle(reticle);
    }
    ~VROInputPresenterDaydream() {}

    void attachElbowNode() {
        _elbowNode = std::make_shared<VRONode>();
        _elbowNode->setSelectable(false);
        _elbowNode->setScale(VROVector3f(.2,  .2,  .2));
        _elbowNode->setPosition(VROVector3f(0.29,  -0.78,  .18));
        getRootNode()->addChildNode(_elbowNode);
    }

    void attachControllerNode() {
        // Textures needed by the controller model representing different controller UI states
        _controllerIdleTexture = std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("ddcontroller_idle.jpg"));
        _controllerAppButtonPressedTexture = std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("ddcontroller_app.jpg"));
        _controllerHomeButtonPressedTexture = std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("ddcontroller_system.jpg"));
        _controllerTouchPadPressedTexture = std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("ddcontroller_touchpad.jpg"));

        // Create Controller Obj Node
        std::string controllerObjAsset = VROPlatformCopyAssetToFile("ddcontroller.obj");
        _controllerNode = VROOBJLoader::loadOBJFromFile(controllerObjAsset, "", false, [this](std::shared_ptr<VRONode> node, bool success) {
            if (!success) {
                perr("ERROR when loading controller obj!");
                return;
            }

            // Set the lighting on this material to be constant
            std::shared_ptr<VROMaterial> &material = node->getGeometry()->getMaterials().front();
            material->getDiffuse().setTexture(_controllerIdleTexture);
        });
        _controllerNode->setSelectable(false);

        // Set it at a predefined position height in relation to the elbow Node
        VROVector3f controllerPosition = _forwardVector * _foreArmLength;
        _controllerNode->setPosition(controllerPosition);

        // Attach it to the model - specifically to the Elbow Node
        _elbowNode->addChildNode(_controllerNode);
    }

    void attachLaserToController() {
        _laserTexture = std::make_shared<VROTexture>(VROPlatformLoadImageFromAsset("ddLaserTexture.jpg"));
        // Create our laser obj
        std::string controllerObjAsset = VROPlatformCopyAssetToFile("ddlaser.obj");
        _pointerNode = VROOBJLoader::loadOBJFromFile(controllerObjAsset, "", false, [this](std::shared_ptr<VRONode> node, bool success) {
            if (!success) {
                perr("ERROR when loading controller obj!");
                return;
            }

            // Set the lighting on this material to be constant
            std::shared_ptr<VROMaterial> &material = node->getGeometry()->getMaterials().front();
            material->setLightingModel(VROLightingModel::Constant);
            material->getDiffuse().setTexture(_laserTexture);
        });

        _pointerNode->setPosition(_controllerNode->getPosition());
        _pointerNode->setOpacity(0.6);
        _pointerNode->setSelectable(false);
        _elbowNode->addChildNode(_pointerNode);
    }

    void setTextureOnController(std::shared_ptr<VROTexture> texture){
        std::shared_ptr<VROMaterial> &material = _controllerNode->getGeometry()->getMaterials().front();
        material->getDiffuse().clear();
        material->getDiffuse().setTexture(texture);
        material->setLightingModel(VROLightingModel::Constant);
    }

    void onClick(int source, ClickState clickState){
        VROInputPresenter::onClick(source, clickState);

        if (source ==ViroDayDream::InputSource::TouchPad && clickState == ClickState::ClickUp){
            getReticle()->trigger();
        }

        /*
         * Update Controller Buttons UI. It is assumed that only
         * one button is being clicked at a time.
         */
        if (clickState == ClickState::ClickUp){
            setTextureOnController(_controllerIdleTexture);
            return;
        }

        if (clickState == ClickState::ClickDown) {
            switch (source) {
                case ViroDayDream::TouchPad:
                    setTextureOnController(_controllerTouchPadPressedTexture);
                    break;
                case ViroDayDream::AppButton:
                    setTextureOnController(_controllerAppButtonPressedTexture);
                    break;
                case ViroDayDream::HomeButton:
                    setTextureOnController(_controllerHomeButtonPressedTexture);
                    break;
            }
        }
    }

    void onTouch(int source, TouchState touchState, float x, float y){
        VROInputPresenter::onTouch(source, touchState, x, y);
        if (touchState == TouchState::TouchUp){
            setTextureOnController(_controllerIdleTexture);
            return;
        }
    }

    void onMove(int source, VROVector3f controllerRotation, VROVector3f controllerPosition){
        VROInputPresenter::onMove(source, controllerRotation, controllerPosition);
        _elbowNode->setRotation(controllerRotation);
    }

    void onGazeHit(int source, float distance, VROVector3f hitLocation){
        VROInputPresenter::onReticleGazeHit(distance, hitLocation);
     }

    std::shared_ptr<VRONode> getControllerPointerNode(){
        return _pointerNode;
    }

    std::shared_ptr<VRONode> getControllerNode(){
        return _controllerNode;
    }

private:
    const float _foreArmLength = 2;
    VROVector3f _forwardVector;

    std::shared_ptr<VRONode> _pointerNode;
    std::shared_ptr<VRONode> _controllerNode;
    std::shared_ptr<VRONode> _elbowNode;

    // Textures representing different daydream states
    std::shared_ptr<VROTexture> _laserTexture;
    std::shared_ptr<VROTexture> _controllerIdleTexture;
    std::shared_ptr<VROTexture> _controllerAppButtonPressedTexture;
    std::shared_ptr<VROTexture> _controllerHomeButtonPressedTexture;
    std::shared_ptr<VROTexture> _controllerTouchPadPressedTexture;
};
#endif
