#ifndef LOG_H_
#define LOG_H_

#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> logger;

void initLogger(const std::string &, const std::string &);

#endif // LOG_H_