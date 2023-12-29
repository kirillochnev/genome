#include <simdjson.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <set>

int main(int argc, const char** argv) {
    if (argc < 4) {
        std::cerr << "Invalid argument count" << std::endl;
        std::exit(1);
    }

    const std::string input_file_name = argv[1];
    const std::string output_directory = argv[2];
    const std::string config_file = argv[3];

    simdjson::dom::parser parser;
    simdjson::dom::parser config_parser;
    std::cout << "Loading [" << input_file_name << "]" << std::endl;
    auto document = parser.load(input_file_name);
    auto error = document.error();
    if (document.error() != simdjson::error_code::SUCCESS) {
        std::cerr << "Error while loading inspector json: " << error << std::endl;
        return 1;
    }

    auto config = config_parser.load(config_file);
    error = config.error();
    if (config.error() != simdjson::error_code::SUCCESS) {
        std::cerr << "Error while loading config json: " << error << std::endl;
        return 1;
    }

    std::set<std::string> headers;
    for (const auto& header : config["headers"]) {
        headers.insert(std::string(header));
    }

    const auto& classes = document["classes"];
    const auto& generators = config["generators"];
    for (const auto& class_to_reflect : classes) {
        const auto header = std::string(class_to_reflect["include_dir"]);
        headers.emplace(header);
    }

    std::ofstream file;
    file.open(output_directory + "/main.cpp");
    file << "// This file was generated\n\n";
    file << "#include <reflection/module_name.hpp>\n";
    for (const auto header : headers) {
        file << "#include <" << header << ">\n";
    }
    file << std::endl;


    for (const auto& class_to_reflect : classes) {
        const auto class_name = std::string(class_to_reflect["name"]);
        file << "//-----------------------  " << class_name << "  -----------------------" << std::endl;
        for (const auto& generator : generators) {
            file << "template<>\nvoid genome::Inspector::processConstObject(const " << class_name
                << "& object, " << std::string(generator) << "& generator) {" << std::endl;
            for (auto field : class_to_reflect["fields"]) {
                auto field_name = field["name"];
                file << "\tgenerator.processConstObject(" << field_name << ", object." << field_name.get_string() << ");" << std::endl;
            }
            file << "}\n" << std::endl;

            file << "template<>\nbool genome::Inspector::processMutableObject(" << class_name
                 << "& object, " << std::string(generator) << "& generator) {" << std::endl;
            file << "\tbool result = false;\n";
            for (auto field : class_to_reflect["fields"]) {
                auto field_name = field["name"];
                file << "\tresult = generator.processMutableObject(" << field_name << ", object." << field_name.get_string() << ") || result;" << std::endl;
            }
            file << "\treturn result;\n";
            file << "}\n" << std::endl;

            file << "template<>\nvoid genome::Inspector::processClass<" << class_name << ">(" << std::string(generator) << "& generator) {" << std::endl;
            for (auto field : class_to_reflect["fields"]) {
                auto field_name = field["name"];
                file << "\tgenerator.processClass<"<< class_name  << ">(" << field_name << ", &" << class_name << "::" << field_name.get_string() << ");" << std::endl;
            }
            file << "}\n" << std::endl;
        }
    }

    const auto module = std::string(document["module"]);
    for (const auto& generator : generators) {
        const auto generator_name = std::string(generator);
        file << "//-----------------------  Register generator " << generator_name << "  -----------------------" << std::endl;
        file << "template<>\nvoid genome::Inspector::registerModule<" << generator_name << ", \"" << module << "\"_module>() {\n";
        for (const auto& class_to_reflect : classes) {
            const auto class_name = std::string(class_to_reflect["name"]);
            file << "\tregisterFunction<" << class_name << ", " << generator_name << ">();\n";
        }
        file << "}\n";
    }
    return 0;
}
