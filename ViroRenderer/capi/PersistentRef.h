//
//  PersistentRef.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef PERSISTENTREF_H
#define PERSISTENTREF_H
#include <iostream>
#include <memory>

/**
 * Wraps a shared pointer object to keep
 * it alive within the life cycle of PersistentRef.
 */
template <class T>
class PersistentRef {
public:

    PersistentRef(std::shared_ptr<T> ptr) : _persistedSharedPtr(ptr){}

    ~PersistentRef() {
        _persistedSharedPtr.reset();
    }

    std::shared_ptr<T> get() const {
        return _persistedSharedPtr;
    }
private:
    std::shared_ptr<T> _persistedSharedPtr;

};

#endif
