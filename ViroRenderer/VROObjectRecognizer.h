//
//  VROObjectRecognizer.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROObjectRecognizer_h
#define VROObjectRecognizer_h

#include <memory>
#include <map>
#include <string>
#include "VROVector3f.h"
#include "VROMatrix4f.h"

enum VRORecognizedObjectType {
    Person,
    Bicycle,
    Car,
    Motorbike,
    Aeroplane,
    Bus,
    Train,
    Truck,
    Boat,
    TrafficLight,
    FireHydrant,
    StopSign,
    ParkingMeter,
    Bench,
    Bird,
    Cat,
    Dog,
    Horse,
    Sheep,
    Cow,
    Elephant,
    Bear,
    Zebra,
    Giraffe,
    Backpack,
    Umbrella,
    Handbag,
    Tie,
    Suitcase,
    Frisbee,
    Skis,
    Snowboard,
    SportsBall,
    Kite,
    BaseballBat,
    BaseballGlove,
    Skateboard,
    Surfboard,
    TennisRacket,
    Bottle,
    WineGlass,
    Cup,
    Fork,
    Knife,
    Spoon,
    Bowl,
    Banana,
    Apple,
    Sandwich,
    Orange,
    Broccoli,
    Carrot,
    HotDog,
    Pizza,
    Donut,
    Cake,
    Chair,
    Sofa,
    PottedPlant,
    Bed,
    DiningTable,
    Toilet,
    TVMonitor,
    Laptop,
    Mouse,
    Remote,
    Keyboard,
    CellPhone,
    Microwave,
    Oven,
    Toaster,
    Sink,
    Refrigerator,
    Book,
    Clock,
    Vase,
    Scissors,
    TeddyBear,
    HairDrier,
    Toothbrush,
};

class VRORecognizedObject {
public:
    
    VRORecognizedObject() : _confidence(0) {}
    VRORecognizedObject(std::string type, VROVector3f screenCoords, double confidence) :
        _type(type),
        _screenCoords(screenCoords),
        _confidence(confidence) {}
    
    const VROVector3f &getScreenCoords() const {
        return _screenCoords;
    }
    std::string getType() const {
        return _type;
    }
    double getConfidence() const {
        return _confidence;
    }
    
private:
    VROVector3f _screenCoords;
    std::string _type;
    double _confidence;
    double _spawnTimeMs;
};

class VROObjectRecognizerDelegate {
public:
    virtual void onObjectsFound(const std::map<std::string, VRORecognizedObject> &objects) = 0;
};

class VROObjectRecognizer {
    
public:
    
    static int getNumClasses();
    static int getIndexOfClass(std::string className);
    static std::string getClassName(int classIndex);

    VROObjectRecognizer() {};
    virtual ~VROObjectRecognizer() {}
    
    virtual void startObjectTracking() = 0;
    virtual void stopObjectTracking() = 0;
    
    void setDelegate(std::shared_ptr<VROObjectRecognizerDelegate> delegate) {
        _objectRecognizerDelegate_w = delegate;
    }
    
protected:
    std::weak_ptr<VROObjectRecognizerDelegate> _objectRecognizerDelegate_w;
    
};

#endif /* VROObjectRecognizer_h */
