#include "logger.h"

namespace ft {

const char* Logger::level_to_str[] = {
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL"
};

Logger::Logger(const char* path) {
    fout_ = fopen(path, "wr");
    if (!fout_)
        std::cout << add_time() <<
            " Can not open output file" << std::endl;
    if (pthread_mutex_init(&logger_lock_, NULL)) {
        char* error_time = add_time();
        fprintf(fout_, "%s Can not create mutex\n", error_time);
        delete error_time;
    }
    buf_[0] = strdup("INFO");
    buf_[1] = strdup("ERROR");
    buf_[2] = strdup("WARNING");
    buf_[3] = strdup("CRITICAL");
}

Logger::~Logger() {
    if (pthread_mutex_destroy(&logger_lock_)) {
        char* error_time = add_time();
        fprintf(fout_, "%s Can not destroy mutex\n", error_time);
        delete error_time;
    }
    if (fclose(fout_))
        std::cout << add_time() <<
            " Can not close output file" << std::endl;
    for (int i = 0; i < MMSG_TYPES; ++i)
        delete buf_[i];
}

char* Logger::itos(int msec) {
    char* str = new char[7];
    str[0] = ' ';
    str[1] = '+';
    int tmp = msec;
    int len = 0;
    for (; tmp; tmp /= 10)
        len++;
    memset(str + 2, '0', 5 - len);
    for (int i = 0; msec; ++i) {
        str[5 - i] = msec % 10 + '0';
        msec /= 10;
    }
    str[6] = 0;
    return str;
}

char* Logger::add_time() {
    struct timeval* tv = new struct timeval;
    struct tm* timeinfo = new struct tm;
    char* buffer = new char[80];

    if (gettimeofday(tv, NULL))
        fprintf(fout_, " Can not get time \n");
    timeinfo = localtime_r(&tv->tv_sec, timeinfo);
    strftime(buffer, 80, "%F %T ", timeinfo);
    char* msec = itos(tv->tv_usec/1000);
    strcat(buffer, msec);
    delete tv;
    delete timeinfo;
    delete msec;
    return buffer;
}

char* Logger::format(const char* str, int i) {
    char* formated_str = add_time();
    strcat(formated_str, " | ");
    strcat(formated_str, buf_[i]);
    strcat(formated_str, " | ");
    // TODO(mcottomn): add __FILE__ and __LINE__
    strcat(formated_str, str);
    strcat(formated_str, "\n");
    return formated_str;
}

void Logger::send(Logger::message_type type, const char* str, ...) {
    va_list vl;
    va_start(vl, str);
    char* formated_str = format(str, type);
    pthread_mutex_lock(&logger_lock_);
    vfprintf(fout_, formated_str, vl);
    fflush(fout_);
    pthread_mutex_unlock(&logger_lock_);
    delete formated_str;
    va_end(vl);
}

}  // namespace ft
