#ifndef LOGGER_LOGGER_H_
# define LOGGER_LOGGER_H_

# define LOG 1
# define ERROR_L 2
# define DELTA_TIME 50
# define MMSG_TYPES 4
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# include <ctime>
# include <iostream>
# include <string>
# include <fstream>

namespace ft {

# define log(self, log_level, message, ...) \
        (self)->send((log_level), "%s:%d | " message , __FILE__, __LINE__, ##__VA_ARGS__ )

# define info(self, message, ...) \
        log(self, ft::Logger::INFO, message, ##__VA_ARGS__ )

class Logger {
 public:
    enum message_type{
        INFO,
        ERROR,
        WARNING,
        CRITICAL
    };

    static const char* level_to_str[];

    explicit Logger(const char* path);
    ~Logger();
    void send(Logger::message_type type, const char* str, ...);

 protected:
    void out();
    char* format(const char* str, int i);
    char* add_time();
    char* itos(int msec);

    pthread_mutex_t logger_lock_;
    char* buf_[MMSG_TYPES];
    FILE* fout_;
};

}  // namespace ft

#endif  // LOGGER_LOGGER_H_
