#include <iostream>
#include "cxxopts.hpp"

int main(int argc, char** argv) {
    cxxopts::Options options("test", "description");

    options.add_options()
        ("h,help", "Help message")
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
    }
    
    return 0;
}