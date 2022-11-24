#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include "cxxopts.hpp"

struct ProjectInfo {
    std::string name;
    std::string filePath;
};

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
    contents << "add_executable(" << cmakeName << " ./src/Main.cpp)";

    file << contents.str();
    file.close();

    // create src directory
    std::string srcDirName = dirName + "/src";
    if (!std::filesystem::create_directories(srcDirName))
        throw std::runtime_error("Could not create directory " + srcDirName);

    // create Main.cpp
    file.open(srcDirName + "/Main.cpp");
    if (!file)
        throw std::runtime_error("Could not create Main.cpp");
    
    contents.str(" ");
    contents << "#include <iostream>\n\n"                           <<
                "int main() {\n"                                    <<
                "   std::cout << \"my project\" << std::endl;\n"    <<
                "}";

    file << contents.str();
    file.close();
}

int main(int argc, char** argv) {
    cxxopts::Options options("setup", "Project setup tool");

    ProjectInfo info;

    options.add_options()
        ("h,help", "Help message")
        ("n,name", "Project name", cxxopts::value<std::string>()->default_value("New Project"))
        ("d,directory", "Project directory", cxxopts::value<std::string>()->default_value("./projects/cpp"));
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    info.name = result["name"].as<std::string>();
    info.filePath = result["directory"].as<std::string>();

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