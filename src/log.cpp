#include "spdlog/sinks/basic_file_sink.h"

#include "log.h"

std::shared_ptr<spdlog::logger> logger;

void initLogger(const std::string &loggerName, const std::string &logFileName) {
    logger = spdlog::basic_logger_mt(loggerName, logFileName);
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::info);
    logger->set_pattern("[%Y-%m-%dT%H:%M:%S.%f] [%t] [%l] %v");
}