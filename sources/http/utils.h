#ifndef UTILS_H_
# define UTILS_H_

#include <iostream>

namespace Http {
bool    IsSeparator(int c);
bool    IsPrint(int c);
size_t  FindLastPrint(const std::string& str);
bool    IsUnrsvdSym(char sym);

} // namespace Http

#endif
