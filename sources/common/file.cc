#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "common/file.h"

bool IsExist(const std::string& path) {
    return !access(path.c_str(), F_OK);
}

bool IsDirectory(const std::string& path) {

    struct stat file_stat;

    if (stat(path.c_str(), &file_stat) < 0)
        return false;
    
    return bool(S_ISDIR(file_stat.st_mode));
}
