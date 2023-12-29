#pragma once

#include <string>
#include <cstdint>

namespace genome {
    namespace reflection {
        using Name = std::string;
        using TypeId = Name;

        struct ClassInfo {

        };

        class IClassProcessor {
        public:
            virtual ~IClassProcessor() = default;
            virtual void processMember(const Name& name, const TypeId& type_id, void* value, size_t array_size = 0) = 0;
            virtual void childBegin(const Name& name) = 0;
            virtual void childEnd() = 0;

            /*template<typename T>
            void writeValue(const Name& name, const T& value) {

            }*/


        };
    }
}
