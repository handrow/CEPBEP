#include "io/file.h"

namespace IO {

File::File(Fd fd): Fd_(fd) {
}

void    File::AddFileFlag(int flag, Error* err) {
    int flags = fcntl(Fd_, F_GETFL);
    if (flags < 0)
        *err = SystemError(errno);
    else if (fcntl(Fd_, F_SETFL, flags | flag) < 0)
        *err = SystemError(errno);
}

void    File::Close() {
    close(Fd_);
}

Fd    File::GetFd() {
    return Fd_;
}

std::string File::Read(USize nbytes, Error* err) {
    ISize rc;
    std::string s;

    s.resize(nbytes);

    rc = read(Fd_, const_cast<char*>(s.data()), nbytes);
    if (rc < 0)
        AssignPtrSafely(err, Error(NET_SYSTEM_ERR, "Syscall error: read"));
    else
        s.resize(rc);
    return s;
}

ISize   File::Write(const std::string& s, Error* err) {
    ISize rc = write(Fd_, const_cast<char*>(s.data()), s.size());

    if (rc < 0)
        AssignPtrSafely(err, Error(NET_SYSTEM_ERR, "Syscall error: write"));
    return rc;
}

IO::File  File::OpenFile(const std::string& path, int oflags, Error* err) {
    IO::File    file;

    file.Fd_ = open(path.c_str(), oflags, 0644);
    if (file.Fd_ < 0) {
        *err = SystemError(errno);
    }
    return file;
}

}  // namespace IO
