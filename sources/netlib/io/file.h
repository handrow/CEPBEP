#ifndef NETLIB_IO_FILE_H_
#define NETLIB_IO_FILE_H_

#include <fcntl.h>

#include <string>
#include <cerrno>
#include <map>

#include "common/types.h"
#include "netlib/io/errors.h"

namespace IO {

class File {
 protected:
    fd_t __fd;

 public:
    explicit    File(fd_t fd = -1);
    void        AddFileFlag(int flag, Error* err);
    void        Close();
    fd_t        GetFd();
    std::string Read(usize nbytes, Error* err = NULL);
    isize       Write(const std::string& s, Error* err = NULL);

    static File  OpenFile(const std::string& path, int oflags, Error* err);
};

}  // namespace IO

#endif  // NETLIB_IO_FILE_H_
