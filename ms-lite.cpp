#include "server_application.h"

int main(int argc, char *argv[]) {
    ServerApplication app("server");
    app.run();
    return EXIT_SUCCESS;
}