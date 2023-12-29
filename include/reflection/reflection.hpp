#pragma once

#include <typeinfo>
#include <reflection/module_name.hpp>
#include <utility>

namespace genome {
    struct Inspector {
        struct ClassInfo {
            void (*process_const_object)(const void* object, void* processor) = nullptr;
            bool (*process_mutable_object)(void* object, void* processor) = nullptr;
            void (*process_class)(void* processor) = nullptr;
            bool isValid() const noexcept {
                return process_const_object != nullptr || process_mutable_object != nullptr || process_class != nullptr;
            }
        };

        template<typename _Class, typename _Processor>
        static void processConstObject(const _Class& object, _Processor& processor);
        
        template<typename _Class, typename _Processor>
        static bool processMutableObject(_Class& object, _Processor& processor);

        template<typename _Class, typename _Processor>
        static void processClass(_Processor& processor);

        template<typename _Class, typename _Processor>
        static void inspect(_Processor& processor);

        template<typename _Class, typename _Processor>
        static void registerFunction() {
            ClassInfo info;
            info.process_const_object = [](const void* object, void* processor) {
                processConstObject < _Class, _Processor>(*reinterpret_cast<const _Class*>(object), *reinterpret_cast<_Processor*>(processor));
            };
            info.process_mutable_object= [](void* object, void* processor) {
                return processMutableObject < _Class, _Processor>(*reinterpret_cast<_Class*>(object), *reinterpret_cast<_Processor*>(processor));
            };
            info.process_class = [](void* processor) {
                processClass < _Class, _Processor>(*reinterpret_cast<_Processor*>(processor));
            };

            addClassInfo(typeid(_Class).hash_code(), typeid(_Processor).hash_code(), info);
        }

        static void addClassInfo(std::size_t object_type_hash, std::size_t processor_type_hash, const ClassInfo& info);
        static ClassInfo findClassInfo(std::size_t object_type_hash, std::size_t processor_type_hash);
   
        template<typename _Processor, uint64_t _Module>
        static void registerModule();
    };

    struct AInspector {
        enum class FunctionType : uint32_t {
            kProcessConstObject,
            kProcessMutableObject,
            kProcessClass
        };

        virtual ~AInspector() = 0;
        virtual void handleUnimplemented(FunctionType function, const char* name, void* object, const void* const_object, const std::type_info& object_type);
    };

    template<typename _Derived>
    struct BaseInspector : public AInspector {

        template<typename T>
        static constexpr std::false_type checkProcessMutableObjectImpl(...);

        template<typename T>
        static constexpr auto checkProcessMutableObjectImpl(std::nullptr_t)
            -> decltype(std::declval<T>().processMutableObjectImpl("", std::declval<T&>()));        
        
        template<typename T>
        static constexpr bool checkProcessMutableObjectImpl() { 
            using ResultType = decltype(checkProcessMutableObjectImpl<T>(nullptr));
            return !std::is_same<ResultType, std::false_type >::value;
        }
        
        template<typename _D, typename T>
        static constexpr std::false_type checkProcessConstObjectImpl(...);

        template<typename _D, typename T>
        static constexpr auto checkProcessConstObjectImpl(std::nullptr_t)
            -> decltype(std::declval<_D>().processConstObjectImpl("", std::declval<T>()));
        
        template<typename T>
        static constexpr bool checkProcessConstObjectImpl() {
            using ResultType = decltype(checkProcessConstObjectImpl<_Derived, T>(nullptr));
            return !std::is_same<ResultType, std::false_type >::value;
        }
                
        template<typename T>
        static constexpr std::false_type checkProcessClassObjectImpl(...);

        template<typename T>
        static constexpr auto checkProcessClassObjectImpl(std::nullptr_t)
            -> decltype(std::declval<T>().processClassImpl("", std::declval<T>()));
        
        template<typename T>
        static constexpr bool checkProcessClassObjectImpl() {
            using ResultType = decltype(checkProcessClassObjectImpl<T>(nullptr));
            return !std::is_same<ResultType, std::false_type >::value;
        }
        
        template<typename T>
        bool processMutableObject(const char* name, T& value) {
            if constexpr (_Derived::template checkProcessMutableObjectImpl<T>()) {
                auto self = static_cast<_Derived*>(this);
                return self->processMutableObjectImpl(name, value);
            } else {
                handleUnimplemented(FunctionType::kProcessMutableObject, name, &value, nullptr, typeid(T));
            }
            return false;
        }  
        template<typename T>
        void processConstObject(const char* name, const T& value) {
            if constexpr (checkProcessConstObjectImpl<T>()) {
                auto self = static_cast<_Derived*>(this);
                self->processConstObjectImpl(name, value);
            } else {
                handleUnimplemented(FunctionType::kProcessConstObject, name, nullptr, &value, typeid(T));
            }
        }
        template<typename _Class, typename _Member>
        void processClass(const char* name, _Member&& member) {
            if constexpr (_Derived::template checkProcessClassObjectImpl<_Member>()) {
                auto self = static_cast<_Derived*>(this);
                self->processClassImpl(name, member);
            } else {
                handleUnimplemented(FunctionType::kProcessClass, name, nullptr, &member, typeid(_Member));
            }
        }
    };
}


#define GENERATE_REFLECTION_FUNCTION friend genome::Inspector;

#define EXTRACT_GENOME()


