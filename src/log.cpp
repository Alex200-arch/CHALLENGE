#include "spdlog/sinks/basic_file_sink.h"

#include "log.h"

std::shared_ptr<spdlog::logger> logger;

void init_logger(const std::string &logger_name, const std::string &log_file_name) {
    logger = spdlog::basic_logger_mt(logger_name, log_file_name);
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::info);
    logger->set_pattern("[%Y-%m-%dT%H:%M:%S.%f] [%t] [%l] %v");
}