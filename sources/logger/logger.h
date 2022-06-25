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

namespace Log {

# define log(self, log_level, message, ...) \
        (self)->Send((log_level), message, /* "%s:%d | " message , FILE, LINE, */ ##__VA_ARGS__)

# define debug(self, message, ...) \
        log(self, Log::Logger::DEBUG, message, ##__VA_ARGS__ )

# define info(self, message, ...) \
        log(self, Log::Logger::INFO, message, ##__VA_ARGS__ )

# define warning(self, message, ...) \
        log(self, Log::Logger::WARNING, message, ##__VA_ARGS__ )

# define error(self, message, ...) \
        log(self, Log::Logger::ERROR, message, ##__VA_ARGS__ )

# define critical(self, message, ...) \
        log(self, Log::Logger::CRITICAL, message, ##__VA_ARGS__ )

class Logger {
 public:
    enum LogLvl{
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    explicit Logger(LogLvl lvl = DEBUG, const std::string& logfile_path = "/dev/stdout");
    ~Logger();
    void Send(LogLvl lvl, const char* str, ...);

    void Open() {
        OutStream_ = fopen(Path_.c_str(), "w");
        if (!OutStream_)
            throw std::runtime_error("fopen() failed");
    }

    void Close() {
        if (OutStream_)
            fclose(OutStream_);
    }

 protected:
    static std::string FormatMessage(const char* str, LogLvl lvl);
    static std::string GetCurrentTime();
    static std::string USToString(int msec);

    static const char* LVL_TO_STR[];
    static const size_t SIZE_OF_DATE_STR;

    LogLvl LogLevel_;
    std::string Path_;
    FILE* OutStream_;
};

}  // namespace ft

#endif  // LOGGER_LOGGER_H_
