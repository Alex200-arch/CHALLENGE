#include <iostream>

#include "cxxopts.hpp"
#include "server_application.h"

int main(int argc, char *argv[]) {
    int port;
    cxxopts::Options options("ms-lite", "message server");
    try {
        options.add_options()("p,port", "server port", cxxopts::value<int>()->default_value("23235"))("h,help", "print usage");
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return EXIT_FAILURE;
        }

        port = result["port"].as<int>();
    }
    catch (const cxxopts::exceptions::exception &e) {
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    server_application app("server", port);
    app.run();
    return EXIT_SUCCESS;
}