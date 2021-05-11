#include "logger.h"

namespace ft {

const char* Logger::level_to_str[] = {
    "DEBUG"
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL",
    NULL
};

Logger::Logger(const char* path, Logger::message_type lvl) : lvl_to_out_(lvl) {
    longest();
    if (!path)
        fout_ = fopen("/dev/stdout", "wr");
    else
        fout_ = fopen(path, "wr");
    if (!fout_)
        error_out("fopen() faild: ");
    if (pthread_mutex_init(&logger_lock_, NULL))
        error_out("Can not create mutex: ");
}

Logger::~Logger() {
    if (pthread_mutex_destroy(&logger_lock_))
        error_out("Can not destroy mutex: ");
    if (fclose(fout_))
        error_out("Can not close file for outstream: ");
}

void Logger::longest() {
    formated_len_ = 0;
    for (int i = 0; level_to_str[i]; ++i )
        formated_len_ = std::max(formated_len_, strlen(level_to_str[i]));
}

void Logger::error_out(const char* msg) {
    std::cerr << add_time() << " logger: ";
    perror(msg);
    std::cerr << std::endl;
}

std::string Logger::itos(int msec) {
    std::string str;

    str.reserve(8);
    str.push_back('+');
    for (int i = 0; i < 6; ++i) {
        str.insert(1, 1, static_cast<char>(msec % 10 + '0'));
        msec /= 10;
    }
    return str;
}

std::string Logger::add_time() {
    struct timeval* tv = new struct timeval;
    struct tm* timeinfo = new struct tm;
    char buffer[80];
    std::string str_time;

    str_time.reserve(60);
    if (gettimeofday(tv, NULL))
        error_out("Can not get time: ");
    timeinfo = localtime_r(&tv->tv_sec, timeinfo);
    strftime(buffer, 80, "%F %T ", timeinfo);
    str_time.append(buffer);
    str_time.append(itos(tv->tv_usec));
    delete tv;
    delete timeinfo;
    return str_time;
}

std::string Logger::format(const char* str, int i) {
    std::string formated_str = add_time();
    int border = formated_len_ - strlen(level_to_str[i]);
    formated_str.append(" || ");
    formated_str.insert(formated_str.size() - 2, border, ' ');
    formated_str.insert(formated_str.size() - (2 + border / 2), level_to_str[i]);
    formated_str.append(str);
    formated_str.append("\n");
    return formated_str;
}

void Logger::send(Logger::message_type type, const char* str, ...) {
    if (type < lvl_to_out_)
        return;
    va_list vl;
    va_start(vl, str);
    std::string formated_str = format(str, type);
    pthread_mutex_lock(&logger_lock_);
    vfprintf(fout_, formated_str.c_str(), vl);
    if (fflush(fout_))
        error_out("fflush faild: ");
    pthread_mutex_unlock(&logger_lock_);
    va_end(vl);
}

}  // namespace ft
