#include "common/string_utils.h"
#include "common/time.h"

#include "logger/logger.h"

namespace Log {

const char* Logger::LVL_TO_STR[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL",
    NULL
};

const size_t Logger::SIZE_OF_DATE_STR = 20;

Logger::Logger(Logger::LogLvl lvl, const std::string& logfile_path) : __min_log_lvl(lvl), __path(logfile_path), __fout(NULL) {
}

Logger::~Logger() {
}

std::string Logger::USToString(int usec) {
    std::string numstr = Convert<std::string>(usec);
    const size_t MAX_US_DIGITS = 6;

    return std::string("+") + std::string(MAX_US_DIGITS - numstr.size(), '0') + numstr;
}

std::string Logger::GetCurrentTime() {
    timeval tv = GetTimeOfDay();
    return FormatTimeToStr("%F %T ", tv.tv_sec) + USToString(tv.tv_usec);
}

std::string Logger::FormatMessage(const char* message, Logger::LogLvl lvl) {
    // const size_t LOG_LVL_MAX_LEN = 8;
    // const size_t log_level_len = strlen(LVL_TO_STR[lvl]);
    // const size_t log_level_padding = LOG_LVL_MAX_LEN - log_level_len;
    std::string fmt_string = "[" + GetCurrentTime() + "] (" + std::string(LVL_TO_STR[lvl]) + ")\n" + message + "\n\n";
    return fmt_string;
}

void Logger::Send(Logger::LogLvl lvl, const char* message, ...) {
    if (lvl >= __min_log_lvl) {
        va_list vl;
        va_start(vl, message);
        vfprintf(__fout, FormatMessage(message, lvl).c_str(), vl);
        if (fflush(__fout))
            throw std::runtime_error("fflush failed");
        va_end(vl);
    }
}

}  // namespace Log
