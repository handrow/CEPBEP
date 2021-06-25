#include "netlib/io/file.h"

namespace IO {

File::File(fd_t fd): __fd(fd) {
}

void    File::AddFileFlag(int flag, Error* err) {
    int flags = fcntl(__fd, F_GETFL);
    if (flags < 0)
        *err = SystemError(errno);
    else if (fcntl(__fd, F_SETFL, flags | flag) < 0)
        *err = SystemError(errno);
}

void    File::Close() {
    close(__fd);
}

fd_t    File::GetFd() {
    return __fd;
}

std::string File::Read(usize nbytes, Error* err) {
    isize rc;
    std::string s;

    s.resize(nbytes);

    rc = read(__fd, const_cast<char*>(s.data()), nbytes);
    if (rc < 0)
        *err = Error(NET_SYSTEM_ERR, "Syscall error: read");
    else
        s.resize(rc);
    return s;
}

isize   File::Write(const std::string& s, Error* err) {
    isize rc = write(__fd, const_cast<char*>(s.data()), s.size());

    if (rc < 0)
        *err = Error(NET_SYSTEM_ERR, "Syscall error: write");
    return rc;
}

}  // namespace IO
