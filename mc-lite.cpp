#include "client_application.h"

int main(int argc, char *argv[]) {
    client_application app("alice");
    app.run();
    return EXIT_SUCCESS;
}