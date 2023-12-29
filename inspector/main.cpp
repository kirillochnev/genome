#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"
#include "action_factory.hpp"
#include <fstream>
#include <iostream>
#include "context.hpp"
#include <filesystem>
#include <map>

using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");
cl::opt<std::string> OutputFilename("o", cl::desc("Specify output filename"), cl::value_desc("filename"));
cl::opt<std::string> ModuleName("m", cl::desc("Specify module name"), cl::value_desc("module"));

std::string getOutputFileName() {
    if (!OutputFilename.empty()) {
        return OutputFilename.getValue();
    }
    return "out.json";
}

std::string getFileNameOfClass(const std::string& file_name) {
    std::filesystem::path path {file_name};
    return path.filename().string();
}

std::string findShortestPath(const Context& context, const std::string& file) {
    if (context.include_dirs.empty()) {
        return file;
    }
    const auto get_relative = [to = file](const std::string& from) {
        std::filesystem::path path_from{ from};
        std::filesystem::path path_to{ to };
        return std::filesystem::relative(to, from).generic_string();
    };

    const auto get_metric = [](const std::string& path) {
        if (path.empty()) {
            return std::numeric_limits<uint32_t>::max();
        }
        return static_cast<uint32_t>(path.size());
    };

    std::string result = get_relative(context.include_dirs.front());
    uint32_t min_metric = get_metric(result);
    for (uint32_t i = 1; i < context.include_dirs.size(); ++i) {
        const auto relative = get_relative(context.include_dirs[i]);
        const auto cur_metric = get_metric(relative);
        if (cur_metric < min_metric) {
            min_metric = cur_metric;
            result = relative;
        }
    }
    return result;
}

void toJson(const Context& context) {
    if (context.result.empty()) {
        return;
    }
    const auto out_file_name = getOutputFileName();

    std::cout << "Saving result into json: " << out_file_name  << std::endl;

    std::cout << "out_file_name = [" << out_file_name << "]" << std::endl;
    std::ofstream out {out_file_name};
    out << "{" << std::endl;
    out << "\t\"module\" : \""<< ModuleName.getValue() << "\"," << std::endl;
    out << "\t\"classes\" : [" << std::endl;


    uint32_t class_count = context.result.size();
    std::map<std::string, std::string> include_headers;
    for (const auto& [_, class_info] : context.result) {
        out << "\t\t{\n";
        auto& include_header = include_headers[class_info.file];
        if (include_header.empty()) {

        }
        uint32_t field_count = class_info.fields.size();
        out << "\t\t\t\"name\" : \"" + class_info.name << "\",\n";
        out << "\t\t\t\"file\" : \"" + getFileNameOfClass(class_info.file) << "\",\n";
        out << "\t\t\t\"include_dir\" : \"" + findShortestPath(context, class_info.file) << "\",\n";
        out << "\t\t\t\"fields\" :[\n";
        for (const auto& field : class_info.fields) {
            out << "\t\t\t\t{\n";
            out << "\t\t\t\t\t\"name\" : \"" << field.name << "\",\n";
            out << "\t\t\t\t\t\"access\" : \"" << field.access << "\"\n";
            out << "\t\t\t\t}";
            if (--field_count) {
                out << ",";
            }
            out << std::endl;
        }
        out << "\t\t\t]\n";
        out << "\t\t}";
        if (--class_count) {
            out << ",";
        }
        out << std::endl;
    }
    out << "\t]" << std::endl;
    out << "}" << std::endl;

}
std::vector<std::string > getIncludeDirs(int argc, const char** argv) {
    std::vector<std::string> result;
    const std::string include_prefix = "-I";
    const auto is_include_dir = [include_prefix](const char* str) {
        for (uint32_t i = 0; i < include_prefix.size(); ++i) {
            if (str[i] != include_prefix[i]) {
                return false;
            }
        }
        return true;
    };

    for (uint32_t i = 0; i < argc; ++i) {
        if (is_include_dir(argv[i])) {
            result.emplace_back(argv[i] + include_prefix.size());
        }
    }

    return result;
}

int main(int argc, const char** argv) {
    Context context;
    context.include_dirs = getIncludeDirs(argc, argv);
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser& OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());

    const auto status = Tool.run(newFrontendActionFactory(&context).get());
    if (status != 0) {
        std::cerr << "Error, status: " << status << std::endl;
        return status;
    }
    toJson(context);
    std::cout << "Done" << std::endl;
    return 0;
}
