#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include "cxxopts.hpp"

struct ProjectInfo {
    std::string name;
    std::string filePath;

    std::string templateDir;
    std::vector<std::string> templates;
};

struct Template {
    // name as used in CMake files
    std::string name;
    std::string repo;
    std::string tag;
    std::string linklib;
    std::string start;
    
    // name as used in file names, could potentially be the same as name
    std::string tempName;
};

Template parse_template(const std::string& templateDir, const std::string& templ) {
    std::ifstream file;
    std::string tname = templateDir + "/" + templ + "/" + templ + ".txt";
    file.open(tname);
    if (!file)
        throw std::runtime_error("Could not open " + tname);

    Template t;
    t.tempName = templ;

    std::string line;
    while (std::getline(file, line)) {
        std::size_t index = line.find("=");

        if (index == std::string::npos)
            continue;

        std::string key = line.substr(0, index);
        std::string value = line.substr(index + 1, line.size());

        if (key == "name")
            t.name = value;
        else if (key == "repo")
            t.repo = value;
        else if (key == "tag")
            t.tag = value;
        else if (key == "linklib")
            t.linklib = value;
        else if (key == "start")
            t.start = value;
        else std::cout << "unknown key " << key << std::endl;
    }

    if (t.name.empty() || t.repo.empty() || t.tag.empty() || t.linklib.empty() || t.start.empty())
        throw std::runtime_error("could not parse template file for " + templ);

    return t;
}

void create_project(const ProjectInfo& info) {
    std::string dirName = info.filePath + "/" + info.name;
    if (std::filesystem::exists(dirName))
        throw std::runtime_error("Project already exists");

    // create project directory
    if (!std::filesystem::create_directories(dirName))
       throw std::runtime_error("Could not create directory " + dirName);

    std::ofstream file;
    std::string cmakeName = info.name;
    cmakeName.erase(std::remove_if(cmakeName.begin(), cmakeName.end(), isspace));

    // create CMakeLists.txt
    file.open(dirName + "/CMakeLists.txt");
    if (!file)
        throw std::runtime_error("Could not create CMakeLists.txt");

    std::stringstream contents;
    contents << "cmake_minimum_required(VERSION 3.19)\n\n";
    contents << "project(" << cmakeName << " VERSION 1.0 LANGUAGES CXX)\n\n";

    if (info.templates.size() > 0)
        contents << "include(FetchContent)\n\n";

    std::vector<Template> temps;

    for (const std::string& templ : info.templates) {
        Template t = parse_template(info.templateDir, templ);
        temps.push_back(t);
        contents << "FetchContent_Declare(\n"
                 << "\t" << t.name << "\n"
                 << "\tGIT_REPOSITORY " << t.repo << "\n"
                 << "\tGIT_TAG " << t.tag << "\n"
                 << ")\nFetchContent_MakeAvailable(" << t.name << ")\n\n";
    }

    contents << "set(CMAKE_CXX_STANDARD 17)\n" << "set(CMAKE_CXX_STANDARD_REQUIRED True)\n\n";
    contents << "set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS True)\n" << "set(BUILD_SHARED_LIBS True)\n\n";

    contents << "add_executable(" << cmakeName << " ./src/Main.cpp)\n";

    contents << "target_link_libraries(" << cmakeName;

    for (const Template& t : temps)
        contents << " " << t.linklib;
    contents << ")\n";

    file << contents.str();
    file.close();

    std::vector<std::string> srcLines;
    for (const Template& t : temps) {
        srcLines.push_back("\n/* " + t.tempName + ".cpp */\n");
        std::ifstream srcfile;
        std::string path = info.templateDir + "/" + t.tempName + "/" + t.tempName + ".cpp";
        srcfile.open(path);

        if (!srcfile)
            throw std::runtime_error("Could not open " + path);

        std::string line;
        while (std::getline(srcfile, line)) {
            srcLines.push_back(line + "\n");
        }
    }

    // create src directory
    std::string srcDirName = dirName + "/src";
    if (!std::filesystem::create_directories(srcDirName))
        throw std::runtime_error("Could not create directory " + srcDirName);

    // create Main.cpp
    file.open(srcDirName + "/Main.cpp");
    if (!file)
        throw std::runtime_error("Could not create Main.cpp");
    
    contents.str(" ");
    contents << "#include <iostream>\n";

    for (const std::string& srcLine : srcLines)
        contents << srcLine;

    contents << "\nint main() {\n"  
             << "\tstd::cout << \"my project\" << std::endl;\n";

    for (const Template& t : temps)
        contents << "\n\t/* " << t.tempName << ".cpp */\n" << "\t" << t.start << "();\n";

    contents << "};";

    file << contents.str();
    file.close();
}

int main(int argc, char** argv) {
    cxxopts::Options options("setup", "Project setup tool");

    ProjectInfo info;

    options.add_options()
        ("h,help", "Help message")
        ("n,name", "Project name", cxxopts::value<std::string>()->default_value("New Project"))
        ("d,directory", "Project directory", cxxopts::value<std::string>()->default_value("C:/projects/cpp"))
        ("td,templatedirectory", "Set template directory", cxxopts::value<std::string>()->default_value("C:/projects/cpp/templates"))
        ("t,templates", "Use templates", cxxopts::value<std::vector<std::string>>())
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    info.name = result["name"].as<std::string>();
    info.filePath = result["directory"].as<std::string>();

    info.templateDir = result["templatedirectory"].as<std::string>();
    info.templates = result["templates"].as<std::vector<std::string>>();

    try {
        create_project(info);

        std::cout << "Succesfully created project" << std::endl;
    }
    catch (std::exception e) {
        std::cout << "Error: " << e.what() << std::endl;
        std::exit(-1);
    }
    
    return 0;
}