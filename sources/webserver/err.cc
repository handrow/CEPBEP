#include "webserver/webserver.h"

namespace Webserver {



void            HttpServer::__SendDefaultErrPage(SessionCtx* ss) {
    ss->ResponseWriter.Write(Convert<std::string>(ss->ResponseCode) + " error\n");
    __OnHttpResponse(ss);
}

IO::File     HttpServer::__GetErrPage(int errcode, SessionCtx* ss) {
    Error err;
    const std::string&  file_path(ss->Server->ErrorPages[errcode]);
    IO::File  file = IO::File::OpenFile(file_path, O_RDONLY, &err);
    if (!err.IsError()) {
        std::string mime_type = Mime::MapType(ss->Server->MimeMap, file_path);

        info(ss->AccessLog, "Session[%d]: sending static error file (%s) with type \"%s\"",
                            ss->ConnectionSock.GetFd(),
                            file_path.c_str(),
                            mime_type.c_str());
        ss->ResponseWriter.Header().Set("Content-type", mime_type);
    }
    return file;
}


}  // Webserver