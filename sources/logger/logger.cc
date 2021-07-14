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

Logger::Logger(Logger::LogLvl lvl, const char* logfile_path) : __min_log_lvl(lvl) {
    __fout = fopen(logfile_path, "w");
    if (!__fout)
        throw std::runtime_error("fopen() failed");
    if (pthread_mutex_init(&__output_mtx, NULL))
        throw std::runtime_error("Can't create mutex");
}

Logger::~Logger() {
    if (pthread_mutex_destroy(&__output_mtx))
        throw std::runtime_error("Can't destroy mutex");
    if (fclose(__fout))
        throw std::runtime_error("Can't close file for outstream");
}

std::string Logger::USToString(int usec) {
    std::string numstr = std::to_string(usec);
    const size_t MAX_US_DIGITS = 6;

    return std::string("+") + std::string(MAX_US_DIGITS - numstr.size(), '0') + numstr;
}

std::string Logger::GetCurrentTime() {
    struct timeval tv;
    struct tm timeinfo;
    std::string str_time(SIZE_OF_DATE_STR, '\0');

    if (gettimeofday(&tv, NULL))
        throw std::runtime_error("Can't get time");
    localtime_r(&(tv.tv_sec), &timeinfo);
    strftime(const_cast<char*>(str_time.data()), SIZE_OF_DATE_STR, "%F %T ", &timeinfo);
    return str_time + USToString(tv.tv_usec);
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
        pthread_mutex_lock(&__output_mtx);
        vfprintf(__fout, FormatMessage(message, lvl).c_str(), vl);
        if (fflush(__fout))
            throw std::runtime_error("fflush failed");
        pthread_mutex_unlock(&__output_mtx);
        va_end(vl);
    }
}

}  // namespace Log
