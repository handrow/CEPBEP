#ifndef COMMON_FILE_H_
#define COMMON_FILE_H_

#include <string>

#include "common/types.h"

bool IsExist(const std::string& path);
bool IsDirectory(const std::string& path);
std::string AppendPath(const std::string& dirname, const std::string& basename);

#endif  // COMMON_FILE_H_
