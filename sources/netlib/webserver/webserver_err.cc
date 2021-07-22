#include "netlib/webserver/webserver.h"

namespace Webserver {



void            HttpServer::__SendDefaultErrPage(SessionCtx* ss) {
    ss->http_writer.Write(Convert<std::string>(ss->res_code) + " error\n");
    __OnHttpResponse(ss);
}

IO::File     HttpServer::__GetErrPage(int errcode, SessionCtx* ss) {
    Error err;
    const std::string&  file_path(ss->server->errpages[errcode]);
    IO::File  file = IO::File::OpenFile(file_path, O_RDONLY, &err);
    if (!err.IsError()) {
        std::string mime_type = Mime::MapType(ss->server->mime_map, file_path);

        info(ss->access_log, "Session[%d]: sending static error file (%s) with type \"%s\"",
                            ss->conn_sock.GetFd(),
                            file_path.c_str(),
                            mime_type.c_str());
        ss->http_writer.Header().Set("Content-type", mime_type);
    }
    return file;
}


}  // Webserver