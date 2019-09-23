//
//  VROObjectRecognizer.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VROObjectRecognizer_h
#define VROObjectRecognizer_h

#include <memory>
#include <map>
#include <string>
#include "VROVisionModel.h"
#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROBoundingBox.h"

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
    VRORecognizedObject(std::string type, VROBoundingBox bounds, double confidence) :
        _type(type),
        _bounds(bounds),
        _confidence(confidence) {}
    
    const VROBoundingBox &getBounds() const {
        return _bounds;
    }
    std::string getType() const {
        return _type;
    }
    double getConfidence() const {
        return _confidence;
    }
    
private:
    VROBoundingBox _bounds;
    std::string _type;
    double _confidence;
    double _spawnTimeMs;
};

class VROObjectRecognizerDelegate {
public:
    virtual void onObjectsFound(const std::map<std::string, std::vector<VRORecognizedObject>> &objects) = 0;
};

class VROObjectRecognizer : public VROVisionModel {
    
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
