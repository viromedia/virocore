#ifndef STRING_JNI_h
#define STRING_JNI_h

#include <jni/object.hpp>
#include <jni/array.hpp>
#include <jni/make.hpp>
#include <jni/npe.hpp>

#include <locale>

namespace jni
   {
    struct StringTag { static constexpr auto Name() { return "java/lang/String"; } };

    template <>
    struct UntaggedObjectType<StringTag> { using Type = jstring; };

    using String = Object<StringTag>;

    // Not this fails on multibyte characters. We only use it because <codecvt> is not
    // supported on Android
    inline std::u16string stringToWString(const std::string &s) {
        std::u16string wsTmp(s.begin(), s.end());
        return wsTmp;
    }
    inline std::string wstringToString(const std::u16string &s) {
        std::string wsTmp(s.begin(), s.end());
        return wsTmp;
    }

    inline std::u16string MakeAnything(ThingToMake<std::u16string>, JNIEnv& env, const String& string)
       {
        NullCheck(env, string.Get());
        std::u16string result(jni::GetStringLength(env, *string), char16_t());
        jni::GetStringRegion(env, *string, 0, result);
        return result;
       }

    inline std::string MakeAnything(ThingToMake<std::string>, JNIEnv& env, const String& string)
       {
        return wstringToString(Make<std::u16string>(env, string));
       }

    inline String MakeAnything(ThingToMake<String>, JNIEnv& env, const std::u16string& string)
       {
        return String(&NewString(env, string));
       }

    inline String MakeAnything(ThingToMake<String>, JNIEnv& env, const std::string& string)
       {
        return Make<String>(env, stringToWString(string));
       }
   }

#endif