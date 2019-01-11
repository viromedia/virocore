//
//  VROObjectRecognizer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/11/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROObjectRecognizer.h"
#include <vector>

static const std::vector<std::string> sClassIdentifiers = {
    "person",
    "bicycle",
    "car",
    "motorbike",
    "aeroplane",
    "bus",
    "train",
    "truck",
    "boat",
    "traffic light",
    "fire hydrant",
    "stop sign",
    "parking meter",
    "bench",
    "bird",
    "cat",
    "dog",
    "horse",
    "sheep",
    "cow",
    "elephant",
    "bear",
    "zebra",
    "giraffe",
    "backpack",
    "umbrella",
    "handbag",
    "tie",
    "suitcase",
    "frisbee",
    "skis",
    "snowboard",
    "sports ball",
    "kite",
    "baseball bat",
    "baseball glove",
    "skateboard",
    "surfboard",
    "tennis racket",
    "bottle",
    "wine glass",
    "cup",
    "fork",
    "knife",
    "spoon",
    "bowl",
    "banana",
    "apple",
    "sandwich",
    "orange",
    "broccoli",
    "carrot",
    "hot dog",
    "pizza",
    "donut",
    "cake",
    "chair",
    "sofa",
    "pottedplant",
    "bed",
    "diningtable",
    "toilet",
    "tvmonitor",
    "laptop",
    "mouse",
    "remote",
    "keyboard",
    "cell phone",
    "microwave",
    "oven",
    "toaster",
    "sink",
    "refrigerator",
    "book",
    "clock",
    "vase",
    "scissors",
    "teddy bear",
    "hair drier",
    "toothbrush",
};

int VROObjectRecognizer::getNumClasses() {
    return (int) sClassIdentifiers.size();
}

int VROObjectRecognizer::getIndexOfClass(std::string classIdentifier) {
    auto it = std::find(sClassIdentifiers.begin(), sClassIdentifiers.end(), classIdentifier);
    if (it == sClassIdentifiers.end()) {
        return -1;
    } else {
        return (int) std::distance(sClassIdentifiers.begin(), it);
    }
}

std::string VROObjectRecognizer::getClassName(int classIndex) {
    return sClassIdentifiers[classIndex];
}
