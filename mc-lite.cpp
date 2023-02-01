#include <iostream>

#include "client_application.h"
#include "cxxopts.hpp"

int main(int argc, char *argv[]) {
    std::string name;
    std::string password;
    cxxopts::Options options("mc-lite", "message client");
    try {
        options.add_options()("n,name", "user name", cxxopts::value<std::string>())("p,password", "user password", cxxopts::value<std::string>())("h,help", "print usage");
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return EXIT_FAILURE;
        }

        if (result.count("name")) {
            name = result["name"].as<std::string>();
        }

        if (result.count("password")) {
            password = result["password"].as<std::string>();
        }
    }
    catch (const cxxopts::exceptions::exception &e) {
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    if (!name.empty()) {
        client_application app(name, password);
        app.run();
    }
    else {
        std::cout << "please assign user name" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}