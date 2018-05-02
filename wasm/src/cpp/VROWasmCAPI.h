//
// Created by Raj Advani on 4/30/18.
//

#ifndef PROJECT_VROWASMINCLUDE_H
#define PROJECT_VROWASMINCLUDE_H

#include <emscripten.h>
#include <emscripten/val.h>

#define VRO_ENV void*
#define VRO_ARGS emscripten::val obj,
#define VRO_ARGS_STATIC VRO_ENV env,
#define VRO_NO_ARGS emscripten::val obj
#define VRO_NO_ARGS_STATIC VRO_ENV env
#define VRO_METHOD_PREAMBLE void *env
#define VRO_REF int
#define VRO_BOOL bool
#define VRO_INT int
#define VRO_LONG uint64_t
#define VRO_FLOAT float
#define VRO_DOUBLE double
#define VRO_CHAR_WIDE wchar_t

#define VRO_OBJECT emscripten::val
#define VRO_OBJECT_NULL emscripten::val::null()
#define VRO_IS_OBJECT_NULL(object) \
    false
#define VRO_WEAK emscripten::val

#define VRO_STRING std::string
#define VRO_NEW_STRING(chars) \
    std::string(chars)
#define VRO_STRING_LENGTH(str) \
    str.size()
#define VRO_STRING_GET_CHARS(str) \
    str.c_str()
#define VRO_STRING_RELEASE_CHARS(str, chars) \

#define VRO_IS_STRING_EMPTY(str) \
    (str.size() == 0)
#define VRO_STRING_STL(str) \
    str

#define VRO_STRING_WIDE std::wstring
#define VRO_IS_WIDE_STRING_EMPTY(str) \
    (str.size() == 0)
#define VRO_STRING_GET_CHARS_WIDE(str, wide_str) \
    wide_str = str

#define VRO_ARRAY(type) std::shared_ptr<std::vector<type>>
#define VRO_ARRAY_LENGTH(array) \
    (int) array->size()
#define VRO_ARRAY_GET(array, index) \
    (*array)[index]
#define VRO_ARRAY_SET(array, index, object) \
    (*array)[index] = object
#define VRO_NEW_ARRAY(size, type, cls) \
    std::make_shared<std::vector<type>>(size)

#define VRO_OBJECT_ARRAY std::shared_ptr<std::vector<emscripten::val>>
#define VRO_OBJECT_ARRAY_GET(array, index) \
    (*array)[index]
#define VRO_OBJECT_ARRAY_SET(array, index, object) \
    (*array)[index] = object
#define VRO_NEW_OBJECT_ARRAY(size, cls) \
    std::make_shared<std::vector<emscripten::val>>(size, emscripten::val::null())

#define VRO_FLOAT_ARRAY std::shared_ptr<std::vector<float>>
#define VRO_NEW_FLOAT_ARRAY(size) \
    std::make_shared<std::vector<float>>(size)
#define VRO_FLOAT_ARRAY_SET(dest, start, len, src) \
    dest->insert(dest->begin() + start, &src[0], &src[len])
#define VRO_FLOAT_ARRAY_GET_ELEMENTS(array) \
    array->data()
#define VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(array, elements) \


#define VRO_DOUBLE_ARRAY std::shared_ptr<std::vector<double>>
#define VRO_NEW_DOUBLE_ARRAY(size) \
    std::make_shared<std::vector<double>>(size)
#define VRO_DOUBLE_ARRAY_SET(dest, start, len, src) \
    dest->insert(dest->begin() + start, &src[0], &src[len])
#define VRO_DOUBLE_ARRAY_GET_ELEMENTS(array) \
    array->data()
#define VRO_DOUBLE_ARRAY_RELEASE_ELEMENTS(array, elements) \


#define VRO_INT_ARRAY std::shared_ptr<std::vector<int>>
#define VRO_LONG_ARRAY std::shared_ptr<std::vector<uint64_t>>
#define VRO_NEW_LONG_ARRAY(size) \
    std::make_shared<std::vector<uint64_t>>(size)
#define VRO_LONG_ARRAY_SET(dest, start, len, src) \
    dest->insert(dest->begin() + start, &src[0], &src[len])
#define VRO_LONG_ARRAY_GET_ELEMENTS(array) \
    array->data()
#define VRO_LONG_ARRAY_RELEASE_ELEMENTS(array, elements) \

#define VRO_STRING_ARRAY std::shared_ptr<std::vector<std::string>>
#define VRO_NEW_STRING_ARRAY(size) \
    std::make_shared<std::vector<std::string>>(size)
#define VRO_STRING_ARRAY_GET(array, index) \
    (*array)[index]
#define VRO_STRING_ARRAY_SET(array, index, item) \
    (*array)[index] = item

#define VRO_NEW_GLOBAL_REF(object) \
    object
#define VRO_NEW_LOCAL_REF(object) \
    object
#define VRO_NEW_WEAK_GLOBAL_REF(object) \
    object

#define VRO_DELETE_GLOBAL_REF(object) \

#define VRO_DELETE_LOCAL_REF(object) \

#define VRO_DELETE_WEAK_GLOBAL_REF(object) \


#define VRO_BUFFER_GET_CAPACITY(buffer) \
    0
#define VRO_BUFFER_GET_ADDRESS(buffer) \
    0

#endif //PROJECT_VROWASMINCLUDE_H
