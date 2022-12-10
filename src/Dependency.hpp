#pragma once

#include <string>

struct Dependency {
    std::string name;

    std::string git_tag;
    std::string repository_link;
};

// common dependencies I use
namespace dependencies {
    const Dependency SFML = Dependency {"SFML", "2.6.x", "https://github.com/SFML/SFML.git"};
};