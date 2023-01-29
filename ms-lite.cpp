#include "server_application.h"

int main(int argc, char *argv[]) {
    server_application app("server");
    app.run();
    return EXIT_SUCCESS;
}