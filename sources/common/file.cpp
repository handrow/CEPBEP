#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "common/file.h"
#include "common/string_utils.h"

bool IsExist(const std::string& path) {
    return !access(path.c_str(), F_OK);
}

bool IsDirectory(const std::string& path) {

    struct stat file_stat;

    if (stat(path.c_str(), &file_stat) < 0)
        return false;
    
    return bool(S_ISDIR(file_stat.st_mode));
}

std::string AppendPath(const std::string& dirname, const std::string& basename) {
    std::string normalized_dirname = (Back(dirname) == '/')
                                     ? dirname
                                     : dirname + "/";

    const USize last_sep_pos = normalized_dirname.find_last_not_of("/") + 1;

    return normalized_dirname.substr(0, last_sep_pos + 1) + basename;
}
