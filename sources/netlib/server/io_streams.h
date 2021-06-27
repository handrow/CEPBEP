#ifndef NETLIB_SERVER_IO_H_
#define NETLIB_SERVER_IO_H_

#include "common/types.h"

#include "http/http.h"
#include "http/reader.h"

#include "netlib/io/file.h"
#include "netlib/io/socket.h"

namespace Server {

struct ListenerStream {
    IO::Socket          ios;
};

struct ConnectionStream {
    IO::Socket          ios;
    Http::RequestReader req_rdr;
    std::string         res_buff;
};

struct StaticFileStream {
    IO::File            ios;
    std::string         wbuffer;
    std::string         rbuffer;
    std::string         path;
};

template <class IO_Serv_Primitive>
fd_t GetFd(const IO_Serv_Primitive& strm) {
    return strm.ios.GetFd();
}

}  // namespace Server

#endif  // NETLIB_SERVER_IO_H_
