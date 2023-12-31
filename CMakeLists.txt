cmake_minimum_required(VERSION 3.27)
project(genome)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/inspector)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/generator)

add_library(reflection_headers STATIC ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_include_directories(reflection_headers PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)


function(get_linked_libraries out_list target)
    if (NOT TARGET ${target})
        return()
    endif()
    get_target_property(LOCAL_RESULT ${target} LINK_LIBRARIES)
    if (NOT LOCAL_RESULT)
        return()
    endif()
    set(LOCAL_RESULT_CPY ${LOCAL_RESULT})
    foreach (item IN LISTS  LOCAL_RESULT_CPY)
        get_linked_libraries(SUB_LIST ${item})
        if (SUB_LIST)
            list(APPEND LOCAL_RESULT ${SUB_LIST})
        endif()
    endforeach()

    set(${out_list} ${LOCAL_RESULT} PARENT_SCOPE)
endfunction()

function(get_headers_of_target out target)
    set(HEAD_EXTENSIONS ".h" ".hpp" ".hxx")
    get_target_property(TARGET_SOURCES ${target} SOURCES)
    set(result "")
    foreach(source_file IN LISTS  TARGET_SOURCES)
        get_filename_component(extension ${source_file} EXT)
        if (${extension} IN_LIST HEAD_EXTENSIONS)
            get_filename_component(absolute_path ${source_file} ABSOLUTE)
            list(APPEND result ${absolute_path})
        endif()
    endforeach()

    set(${out} ${result} PARENT_SCOPE)
endfunction()

function(get_include_directories_of_target out target)
    set(result "")
    get_linked_libraries(lib_list ${target})
    foreach(lib IN LISTS lib_list)
        if (TARGET ${lib})
            get_target_property(sub_list ${lib} INCLUDE_DIRECTORIES)
            if (sub_list)
                list(APPEND result ${sub_list})
            endif()
            get_target_property(sub_list ${lib} INTERFACE_INCLUDE_DIRECTORIES)
            if (sub_list)
                list(APPEND result ${sub_list})
            endif()
        endif()
    endforeach()
    list(REMOVE_DUPLICATES result)
    set(${out} ${result} PARENT_SCOPE)
endfunction()

function(add_reflection_support target)
    set(options "")
    set(oneValueArgs CONFIG)
    set(multiValueArgs GENERATORS HEADERS LIBS)
    cmake_parse_arguments(PARSED_ARGS "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )
    set(generated_folder_name "${CMAKE_BINARY_DIR}/generated")
    if(NOT PARSED_ARGS_CONFIG)
        set(PARSED_ARGS_CONFIG "${generated_folder_name}/cfg.json")
        write_file(${PARSED_ARGS_CONFIG} "{\n\t\"generators\" : [")
        set(INDEX 0)
        list(LENGTH PARSED_ARGS_GENERATORS LAST_INDEX)
        foreach(generator IN LISTS PARSED_ARGS_GENERATORS)
            math(EXPR INDEX "${INDEX} + 1")
            if (${INDEX} EQUAL ${LAST_INDEX})
                write_file(${PARSED_ARGS_CONFIG} "\t\t\"${generator}\"" APPEND)
            else()
                write_file(${PARSED_ARGS_CONFIG} "\t\t\"${generator}\"," APPEND)
            endif()
        endforeach()
        write_file(${PARSED_ARGS_CONFIG} "\t\]," APPEND)

        write_file(${PARSED_ARGS_CONFIG} "\t\"headers\" : [" APPEND)
        if (PARSED_ARGS_HEADERS)
            set(INDEX 0)
            list(LENGTH PARSED_ARGS_HEADERS LAST_INDEX)
            foreach(header IN LISTS PARSED_ARGS_HEADERS)
                math(EXPR INDEX "${INDEX} + 1")
                if (${INDEX} EQUAL ${LAST_INDEX})
                    write_file(${PARSED_ARGS_CONFIG} "\t\t\"${header}\"" APPEND)
                else()
                    write_file(${PARSED_ARGS_CONFIG} "\t\t\"${header}\"," APPEND)
                endif()
            endforeach()
        endif()
        write_file(${PARSED_ARGS_CONFIG} "\t]\n}" APPEND)
    endif()

    target_link_libraries(${target} reflection_headers)
    get_include_directories_of_target(inc_list ${target})
    set(inc_dirs_args "")
    foreach(dir IN LISTS inc_list)
        string(REPLACE "$<BUILD_INTERFACE:" "" dir ${dir})
        string(REPLACE "$<INSTALL_INTERFACE:" "" dir ${dir})
        string(REPLACE ">" "" dir ${dir})
        set(inc_dirs_args ${inc_dirs_args} "-I${dir}")
    endforeach()

    get_headers_of_target(headers ${target})
    if (NOT headers)
        return()
    endif()

    set(custom_target_name "generate_reflection_for_${target}")
    set(generated_folder_name_for_target "${generated_folder_name}/${target}")
    set(out_file_name "${generated_folder_name_for_target}/inspector_result.json")
    file(MAKE_DIRECTORY "${generated_folder_name_for_target}")
    set(generator_file_name "${CMAKE_SOURCE_DIR}/tools/reflection/generator/main.lua")
    set(result_file_name "${generated_folder_name_for_target}/main.cpp")
    #	message(FATAL_ERROR "PARSED_ARGS_CONFIG = [${PARSED_ARGS_CONFIG}]")
    if (NOT EXISTS ${result_file_name})
        file(TOUCH ${result_file_name})
    endif()
    if (0)
        add_custom_target(${custom_target_name} ALL
                reflection_inspector ${headers} -m ${target} -o ${out_file_name} -- -std=c++17 ${inc_dirs_args} COMMAND
                reflection_generator ${out_file_name} "${generated_folder_name_for_target}" "${PARSED_ARGS_CONFIG}"
        )
    endif()

    set(reflection_lib_name "${target}_reflection_lib")
    add_library(${reflection_lib_name} STATIC ${result_file_name})
    target_include_directories(${reflection_lib_name} PRIVATE ${inc_list})
    add_dependencies(${reflection_lib_name} ${custom_target_name})
    add_dependencies(${target} ${reflection_lib_name})
    target_link_libraries(${target} ${reflection_lib_name})

endfunction()
