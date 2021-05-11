#include "logger.h"

namespace ft {

const char* Logger::level_to_str[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL",
    NULL
};

Logger::Logger(Logger::LogLvl lvl, const char* logfile_path) : min_log_level_(lvl) {
    fout_ = fopen(logfile_path, "wr");
    if (!fout_)
        PutError("fopen() faild: ");
    if (pthread_mutex_init(&output_mtx_, NULL))
        PutError("Can not create mutex: ");
}

Logger::~Logger() {
    if (pthread_mutex_destroy(&output_mtx_))
        PutError("Can not destroy mutex: ");
    if (fclose(fout_))
        PutError("Can not close file for outstream: ");
}

void Logger::PutError(const char* msg) {
    std::cerr << GetCurrentTime() << " logger: ";
    perror(msg);
    std::cerr << std::endl;
}

const std::string Logger::MSToString(int msec) {
    std::string str;

    str.reserve(8);
    str.push_back('+');
    for (int i = 0; i < 6; ++i) {
        str.insert(1, 1, static_cast<char>(msec % 10 + '0'));
        msec /= 10;
    }
    return str;
}

const std::string Logger::GetCurrentTime() {
    struct timeval tv;
    struct tm timeinfo;
    char buffer[21];
    std::string str_time;

    str_time.reserve(60);
    if (gettimeofday(&tv, NULL))
        PutError("Can not get time: ");
    localtime_r(&(tv.tv_sec), &timeinfo);
    strftime(buffer, 21, "%F %T ", &timeinfo);
    str_time.append(buffer);
    str_time.append(MSToString(tv.tv_usec));
    return str_time;
}

const std::string Logger::FormatMessage(const char* message, Logger::LogLvl lvl){
    const size_t log_level_len = strlen(level_to_str[lvl]);
    const size_t log_level_padding = 8 - log_level_len;

    std::string fmt_string = GetCurrentTime()
                       + std::string(" | ") + level_to_str[lvl] + std::string(log_level_padding, ' ')
                       + std::string(" | ") + message + "\n";
    return fmt_string;
}

void Logger::Send(Logger::LogLvl lvl, const char* message, ...) {
    if (lvl >= min_log_level_) {
        va_list vl;
        va_start(vl, message);
        std::string formated_str = FormatMessage(message, lvl);
        pthread_mutex_lock(&output_mtx_);
        vfprintf(fout_, formated_str.c_str(), vl);
        if (fflush(fout_))
            PutError("fflush faild: ");
        pthread_mutex_unlock(&output_mtx_);
        va_end(vl);
    }
}

}  // namespace ft
