#include "webserver/webserver.h"

namespace Webserver {



void            HttpServer::SendDefaultErrPage(SessionCtx* ss) {
    ss->ResponseWriter.Write(Convert<std::string>(ss->ResponseCode) + " error\n");
    OnHttpResponse(ss);
}

IO::File     HttpServer::GetErrPage(int errcode, SessionCtx* ss) {
    Error err;
    const std::string&  filePath(ss->Server->ErrorPages[errcode]);
    IO::File  file = IO::File::OpenFile(filePath, O_RDONLY, &err);
    if (!err.IsError()) {
        std::string mimeType = Mime::MapType(ss->Server->MimeMap, filePath);

        info(ss->AccessLog, "Session[%d]: sending static error file (%s) with type \"%s\"",
                            ss->ConnectionSock.GetFd(),
                            filePath.c_str(),
                            mimeType.c_str());
        ss->ResponseWriter.Header().Set("Content-type", mimeType);
    }
    return file;
}


}  // Webserver