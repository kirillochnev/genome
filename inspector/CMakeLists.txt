cmake_minimum_required(VERSION 3.10)

project(reflection_inspector)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    add_definitions(
            /D_SCL_SECURE_NO_WARNINGS
            /D_CRT_SECURE_NO_WARNINGS
    )

    # These are to allow clang/llvm to compile without hogging the output window
    add_definitions(
            /wd4146          # unary minus operator applied to unsigned type, result still unsigned
            /wd4624          # 'clang::Preprocessor::MacroInfoChain' : destructor could not be generated because a base class destructor is inaccessible
            /wd4800          # 'clang::NestedNameSpecifier *const ' : forcing value to bool 'true' or 'false' (performance warning)
            /wd4355          # 'this' : used in base member initializer list
            /wd4291          # 'void *operator new(size_t,void *) throw()' : no matching operator delete found; memory will not be freed if initialization throws an exception
            /wd4244          # 'return' : conversion from 'uint64_t' to 'unsigned int', possible loss of data
    )
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
find_package(Clang REQUIRED CONFIG HINTS "${LLVM_INSTALL_PREFIX}/lib/cmake/clang")

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})


link_directories(${LLVM_LIBRARY_DIR})

set(LLVM_LINK_COMPONENTS support)

add_library(art_reflect INTERFACE)
target_include_directories(art_reflect INTERFACE include)

add_executable(${PROJECT_NAME}
        main.cpp
        macro_callback.hpp
        macro_callback.cpp
        action_factory.hpp
        action_factory.cpp
        ast_callback.hpp
        ast_callback.cpp
)


llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_TARGETS_TO_BUILD}
    core support option
)


target_link_libraries(${PROJECT_NAME}
        clangAST
        clangBasic
        clangCodeGen
        clangLex
        clangParse
        clangSema
        clangAnalysis
        clangDriver
        clangFrontend
        clangAST
        clangEdit
        clangSerialization
        clangTooling
        clangToolingCore
        clangAST
        clangParse
        clangBasic
        clangSema
        clangAST
        ${REQ_LLVM_LIBRARIES}
)
