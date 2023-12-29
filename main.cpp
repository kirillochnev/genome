#include "reflection/reflection.hpp"
#include <map>
#include <iostream>

using namespace genome;

namespace {
    using Info = std::pair<std::size_t, std::size_t>;
    std::map<Info, Inspector::ClassInfo> registred_class_infos;
}

void Inspector::addClassInfo(size_t object_type_hash, size_t processor_type_hash, const ClassInfo& info) {
    registred_class_infos[{object_type_hash, processor_type_hash}] = info;
}

Inspector::ClassInfo Inspector::findClassInfo(size_t object_type_hash, size_t processor_type_hash) {
    const auto find_res = registred_class_infos.find({ object_type_hash, processor_type_hash });
    if (find_res != registred_class_infos.end()) {
        return find_res->second;
    }
    return {};
}


AInspector::~AInspector() = default;

void AInspector::handleUnimplemented(FunctionType function, const char*, void*, const void*, const std::type_info& object_type) {
    const auto& inspector = typeid(this);
    switch (function) {
    case FunctionType::kProcessConstObject:
        std::cerr << "Can not find " << inspector.name() << "::processConstObjectImpl(const char*, const " << object_type.name() << "&);" << std::endl;
        break;
    case FunctionType::kProcessMutableObject:
        std::cerr << "Can not find " << inspector.name() << "::processMutableObjectImpl(const char*, const " << object_type.name() << "&);" << std::endl;
        break;
    case FunctionType::kProcessClass:
        std::cerr << "Can not find " << inspector.name() << "::processClassImpl(const char*, const " << object_type.name() << "&);" << std::endl;
        break;
    default:
        break;
    }
    std::exit(1);
}
