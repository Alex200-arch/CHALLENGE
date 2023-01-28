#include "log.h"

int main(int argc, char *argv[]) {
    initLogger("mc-lite-logger", "mc-lite.log");

    logger->info("hello");
    logger->error("hello");

    return EXIT_SUCCESS;
}