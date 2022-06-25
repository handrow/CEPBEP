#include "common/file.h"
#include "netlib/webserver/webserver.h"

namespace Webserver {

bool  HttpServer::__IsUpload(SessionCtx* ss, const WebRoute& rt) {
    std::string type = ss->Request.Headers.Get("Content-Type");

    return rt.UploadEnabled &&
           type.compare(0, 19, "multipart/form-data") == 0 &&
           ss->Request.Method == Http::METHOD_POST;
}

bool  GetParamTokens(const std::string& param, std::string* key, std::string* val, char d = '=') {
    USize delim = param.find(d);
    if (delim == std::string::npos) {
        return false;
    }

    *key = Trim(param.substr(0, delim), ' ');
    *val = Trim(param.substr(delim + 1), ' ');
    return true;
}

std::string  GetBoundary(const std::string& hval, bool* found) {
    Tokenizator tkz(hval);
    bool run = true;
    std::string token;

    while (token = tkz.Next("; \t", &run), run == true) {
        std::string key, value;

        if (GetParamTokens(token, &key, &value) == false)
            continue;

        if (key == "boundary")
            return *found = true, value;
    }

    return *found = false, "";
}

void  ParseContentDisposition(const std::string& line, std::string* filename) {
    Tokenizator tkz(line);
    bool run = true;
    std::string token;

    token = tkz.Next(";", &run);
    if (run) {
        std::string hkey;
        std::string hvalue;
        if (!GetParamTokens(token, &hkey, &hvalue, ':'))
            return;
        if (StrToLower(hkey) != "content-disposition")
            return;

        if (hvalue != "form-data")
            return;

        while (token = tkz.Next(";", &run), run == true) {
            std::string key;
            std::string value;
            if (!GetParamTokens(token, &key, &value, '='))
                continue;

            value = Trim(value, '"');


            if (key == "filename") {
                *filename = value;
            }
        }
    }
}

void  HandleChunk(const std::string& chunk, std::list<HttpServer::UploadReq>* files) {
    Tokenizator tkz(chunk);

    bool         firstLineFlag = true;
    std::string  filename;

    for (;;) {
        std::string  meta_line;
        bool run = true;
        meta_line = tkz.NextLine(&run);

        if (run == false)
            return;

        if (meta_line.empty() && !firstLineFlag)
            break;

        ParseContentDisposition(meta_line, &filename);
        firstLineFlag = false;
    }
    if (!filename.empty()) {
        USize bodyBegin = tkz.GetPos();
        USize bodyEnd;
        bodyEnd = chunk.rfind("\n");
        if (bodyEnd == std::string::npos)
            bodyEnd = chunk.rfind("\r\n");

        HttpServer::UploadReq upld;
        upld.FileName = filename;
        upld.FileContent = chunk.substr(bodyBegin, bodyEnd - bodyBegin);
        files->push_back(upld);
    }
}

void  HttpServer::__OnUploadEnd(SessionCtx* ss, const WebRoute& route, const std::list<UploadReq>& files) {

    std::string uploadDir = route.RootDir;

    std::list<UploadReq>::const_iterator itend = files.end();
    std::list<UploadReq>::const_iterator it;

    printf("Ya tvar 1\n");

    for (it = files.begin(); it != itend; ++it) {
        std::string upload_path = AppendPath(uploadDir, it->FileName);
        if (IsExist(upload_path)) {
            debug(ss->ErrorLOg, "Session[%d]: file %s already exists, returning 403 error",
                                   ss->ConnectionSock.GetFd(),
                                   upload_path.c_str());
            return ss->ResponseCode = 403, __OnHttpError(ss);
        }
    }

    printf("Ya tvar 2\n");

    bool fileFailed = false;
    std::string fileUrls = "";

    for (it = files.begin(); it != itend; ++it) {
        std::string uploadPath = AppendPath(uploadDir, it->FileName);
        std::string uploadURL = AppendPath(route.Pattern, it->FileName);
    
        Error err;
        IO::File file = IO::File::OpenFile(uploadPath,
                                           O_CREAT | O_EXCL | O_WRONLY,
                                           &err);
        if (err.IsError()) {
            debug(ss->ErrorLOg, "Session[%d]: file %s open failed: %s",
                                   ss->ConnectionSock.GetFd(),
                                   uploadPath.c_str(),
                                   err.Description.c_str());
            fileFailed = true;
        } else {
            file.Write(it->FileContent);
            file.Close();
            fileUrls += uploadURL + "\n";
            debug(ss->ErrorLOg, "Session[%d]: file %s created",
                                   ss->ConnectionSock.GetFd(),
                                   uploadPath.c_str());
        }
    }

    printf("Ya tvar 3\n");

    if (fileFailed) {
        return ss->ResponseCode = 500, __OnHttpError(ss);
    }

    ss->ResponseCode = 201;
    ss->ResponseWriter.Write(fileUrls);
    return __OnHttpResponse(ss);
}

void  HttpServer::__HandleUploadRequest(SessionCtx* ss, const WebRoute& route) {
    info(ss->AccessLog, "Session[%d]: upload request", ss->ConnectionSock.GetFd());

    std::list<UploadReq> files;

    bool  boundaryExists;
    std::string  boundareToken =
            "--" + GetBoundary(ss->Request.Headers.Get("Content-Type"), &boundaryExists);

    if (boundaryExists) {
        Tokenizator tkz(ss->Request.Body);
        bool run = true;
        std::string chunk_token;
        for (;;) {
            chunk_token = tkz.NextS(boundareToken, &run);
            if (run == false)
                break;
            if (chunk_token == "--" || chunk_token == "--\n" || chunk_token == "--\r\n")
                break;
            HandleChunk(chunk_token, &files);
        }
        return __OnUploadEnd(ss, route, files);
    } else {
        debug(ss->ErrorLOg, "Session[%d]: multipart form data boundary isn't exist, returning error", ss->ConnectionSock.GetFd());
        return ss->ResponseCode = 400, __OnHttpError(ss);
    }
}

}  // namespace Webserver
