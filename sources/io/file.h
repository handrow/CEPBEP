#ifndef NETLIB_IO_FILE_H_
#define NETLIB_IO_FILE_H_

#include <fcntl.h>

#include <string>
#include <cerrno>
#include <map>

#include "common/types.h"
#include "io/errors.h"

namespace IO {

class File {
 protected:
    Fd Fd_;

 public:
    explicit    File(Fd fd = -1);
    void        AddFileFlag(int flag, Error* err);
    void        Close();
    Fd          GetFd();
    std::string Read(USize nbytes, Error* err = NULL);
    ISize       Write(const std::string& s, Error* err = NULL);

    static File  OpenFile(const std::string& path, int oflags, Error* err);
};

}  // namespace IO

#endif  // NETLIB_IO_FILE_H_
