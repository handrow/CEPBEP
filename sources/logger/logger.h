#ifndef LOGGER_LOGGER_H_
# define LOGGER_LOGGER_H_

# include <unistd.h>
# include <sys/time.h>

# include <cstdio>
# include <cstdlib>
# include <ctime>
# include <iostream>
# include <string>
# include <fstream>

namespace ft {

# define log(self, log_level, message, ...) \
        (self)->send((log_level), "%s:%d | " message , __FILE__, __LINE__, ##__VA_ARGS__ )

# define info(self, message, ...) \
        log(self, ft::Logger::INFO, message, ##__VA_ARGS__ )

# define warning(self, message, ...) \
        log(self, ft::Logger::WARNING, message, ##__VA_ARGS__ )

# define error(self, message, ...) \
        log(self, ft::Logger::ERROR, message, ##__VA_ARGS__ )

# define critical(self, message, ...) \
        log(self, ft::Logger::CRITICAL, message, ##__VA_ARGS__ )

class Logger {
 public:
    enum message_type{
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    static const char* level_to_str[];

    explicit Logger(const char* path, message_type lvl);
    ~Logger();
    void send(message_type type, const char* str, ...);

 protected:
    void out();
    void longest();
    std::string format(const char* str, int i);
    std::string add_time();
    std::string itos(int msec);

    message_type lvl_to_out_;
    pthread_mutex_t logger_lock_;
    FILE* fout_;
    size_t formated_len_;
};

}  // namespace ft

#endif  // LOGGER_LOGGER_H_
