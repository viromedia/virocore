//
//  VROStringUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROStringUtil.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <iomanip>
#include <cctype>
#include <locale>
#include "VRODefines.h"
#include "VROLog.h"

std::string VROStringUtil::toString(int i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}

std::string VROStringUtil::toString64(uint64_t i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}

std::string VROStringUtil::toString(double n, int precision) {
    std::ostringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss.precision(precision);
    
    ss << n;
    return ss.str();
}

std::wstring VROStringUtil::toWString(int i) {
    std::wstringstream ss;
    ss << i;
    return ss.str();
}

std::wstring VROStringUtil::toWString(double n, int precision) {
    std::wostringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss.precision(precision);
    
    ss << n;
    return ss.str();
}

int VROStringUtil::toInt(std::string s) {
    return atoi(s.c_str());
}

float VROStringUtil::toFloat(std::string s) {
    return atof(s.c_str());
}

std::vector<std::string> VROStringUtil::split(const std::string &s,
                                              const std::string &delimiters,
                                              bool emptiesOk) {
    
    std::vector<std::string> result;
    size_t current;
    size_t next = -1;
    
    do {
        if (!emptiesOk) {
            next = s.find_first_not_of(delimiters, next + 1);
            if (next == std::string::npos) {
                break;
            };
            
            next -= 1;
        }
        
        current = next + 1;
        next = s.find_first_of(delimiters, current);
        result.push_back(s.substr(current, next - current));
    }
    while (next != std::string::npos);
    
    return result;
}

std::vector<std::wstring> VROStringUtil::split(const std::wstring &s,
                                               const std::wstring &delimiters,
                                               bool emptiesOk) {
    
    std::vector<std::wstring> result;
    size_t current;
    size_t next = -1;
    
    do {
        if (!emptiesOk) {
            next = s.find_first_not_of(delimiters, next + 1);
            if (next == std::wstring::npos) {
                break;
            };
            
            next -= 1;
        }
        
        current = next + 1;
        next = s.find_first_of(delimiters, current);
        result.push_back(s.substr(current, next - current));
    }
    while (next != std::wstring::npos);
    
    return result;
}

bool VROStringUtil::strcmpinsensitive(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (int i = 0; i < a.size(); i++) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }
    return true;
}

void VROStringUtil::toLowerCase(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

bool VROStringUtil::startsWith(const std::string &candidate, const std::string &prefix) {
    if (candidate.length() < prefix.length()) {
        return false;
    }
    return 0 == candidate.compare(0, prefix.length(), prefix, 0, prefix.length());
}

bool VROStringUtil::endsWith(const std::string& candidate, const std::string& ending) {
    if (candidate.length() < ending.length()) {
        return false;
    }
    return 0 == candidate.compare(candidate.length() - ending.length(), ending.length(), ending);
}

bool VROStringUtil::replace(std::string &str, const std::string &from, const std::string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) {
        return false;
    }
    str.replace(start_pos, from.length(), to);
    return true;
}

void VROStringUtil::replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty()) {
        return;
    }
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::string charToHex(unsigned char c) {
    short i = c;
    std::stringstream s;
    s << "%" << std::setw(2) << std::setfill('0') << std::hex << i;
    return s.str();
}

std::string VROStringUtil::escapeReservedURLCharacters(std::string text) {
    std::ostringstream out;
    
    // All input characters that are not a-z, A-Z, 0-9, '-', '.', '_' or '~'
    // are converted to their "URL escaped" version
    for (std::string::size_type i=0; i < text.length(); ++i) {
        short t = text.at(i);
        if (t == 45 ||                // hyphen
            t == 46 ||                // period
           (t >= 48 && t <= 57) ||    // 0-9
           (t >= 65 && t <= 90) ||    // A-Z
            t == 95 ||                // underscore
           (t >= 97 && t <= 122) ||   // a-z
            t == 126) {               // tilde
            out << text.at(i);
        } else {
            out << charToHex(text.at(i));
        }
    }
    return out.str();
}

std::string VROStringUtil::escapeSpaces(std::string text) {
    std::ostringstream out;
    for (std::string::size_type i=0; i < text.length(); ++i) {
        short t = text.at(i);
        if (t == 32) {
            out << charToHex(text.at(i));
        } else {
            out << text.at(i);
        }
    }
    return out.str();
}

std::vector<std::string> VROStringUtil::parseURL(std::string url) {
    std::regex regexURL(R"(^([^:\/?#]+:?//[^\/?#]*)?([^?#]*)(\?[^#]*)?(#.*)?)", std::regex::extended);
    unsigned counter = 0;
    std::vector<std::string> components;
    
    std::smatch match;
    if (std::regex_match(url, match, regexURL)) {
        for (const auto &res : match) {
            std::stringstream s; s << res;
            std::string component = s.str();
        
            if (counter > 0) {
                components.push_back(component);
            }
            // Commented out below: useful for debugging
            // pinfo("%d: %s", counter, component.c_str());
            counter++;
        }
    }
    return components;
}

std::string VROStringUtil::encodeURL(std::string url) {
    std::vector<std::string> components = parseURL(url);
    if (components.empty()) {
        pinfo("Failed to encode URL [%s], URL is malformed", url.c_str());
        return url;
    }
    
    std::stringstream s;
    for (int i = 0; i < components.size(); i++) {
        if (i == 1) {
            // Path components need spaces escaped
            s << escapeSpaces(components[i]);
        } else if (i == 2) {
            // Currently do nothing with the query string; in the future will want to escape
            // all reserved characters between the ?, &, and = delimeters
            s << components[i];
        } else {
            s << components[i];
        }
    }
    return s.str();
}

void VROStringUtil::printCode(std::string &code) {
    std::vector<std::string> lines = split(code, "\n", true);
    
    int lineNumber = 0;
    for (std::string &line : lines) {
        ++lineNumber;
        pinfo("%d: %s", lineNumber, line.c_str());
    }
}

// trim from start (in place)
static void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static void trim_inplace(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from both ends (copying)
std::string VROStringUtil::trim(std::string s) {
    trim_inplace(s);
    return s;
}

