#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <clang/Basic/SourceLocation.h>

struct Context {

    struct ReflectRequest {
        std::string file;
        std::string name;
    };
    struct FieldInfo {
        std::string name;
        std::string access;
    };

    struct ClassInfo {
        std::string name;
        std::string file;
        std::vector<FieldInfo> fields;
        std::vector<FieldInfo> static_fields;
    };

    std::vector<std::string> include_dirs;
    std::map<std::string, ReflectRequest> requests;
    std::map<std::string, ClassInfo> result;
};
