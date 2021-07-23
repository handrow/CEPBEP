#include "common/file.h"
#include "netlib/webserver/webserver.h"

namespace Webserver {

bool  HttpServer::__IsUpload(SessionCtx* ss, const WebRoute& rt) {
    std::string type = ss->http_req.headers.Get("Content-Type");

    return rt.upload_enabled &&
           type.compare(0, 19, "multipart/form-data") == 0 &&
           ss->http_req.method == Http::METHOD_POST;
}

bool  GetParamTokens(const std::string& param, std::string* key, std::string* val, char d = '=') {
    usize delim = param.find(d);
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

    bool         first_line = true;
    std::string  filename;

    for (;;) {
        std::string  meta_line;
        bool run = true;
        meta_line = tkz.NextLine(&run);

        if (run == false)
            return;

        if (meta_line.empty() && !first_line)
            break;

        ParseContentDisposition(meta_line, &filename);
        first_line = false;
    }
    if (!filename.empty()) {
        usize body_begin = tkz.GetPos();
        usize body_end;
        body_end = chunk.rfind("\n");
        if (body_end == std::string::npos)
            body_end = chunk.rfind("\r\n");

        HttpServer::UploadReq upld;
        upld.filename = filename;
        upld.file_content = chunk.substr(body_begin, body_end - body_begin);
        files->push_back(upld);
    }
}

void  HttpServer::__OnUploadEnd(SessionCtx* ss, const WebRoute& route, const std::list<UploadReq>& files) {

    std::string upload_dir = route.root_directory;

    std::list<UploadReq>::const_iterator itend = files.end();
    std::list<UploadReq>::const_iterator it;

    printf("Ya tvar 1\n");

    for (it = files.begin(); it != itend; ++it) {
        std::string upload_path = AppendPath(upload_dir, it->filename);
        if (IsExist(upload_path)) {
            debug(ss->error_log, "Session[%d]: file %s already exists, returning 403 error",
                                   ss->conn_sock.GetFd(),
                                   upload_path.c_str());
            return ss->res_code = 403, __OnHttpError(ss);
        }
    }

    printf("Ya tvar 2\n");

    bool file_fail = false;
    std::string file_urls = "";

    for (it = files.begin(); it != itend; ++it) {
        std::string upload_path = AppendPath(upload_dir, it->filename);
        std::string upload_url = AppendPath(route.pattern, it->filename);
    
        Error err;
        IO::File file = IO::File::OpenFile(upload_path,
                                           O_CREAT | O_EXCL | O_WRONLY,
                                           &err);
        if (err.IsError()) {
            debug(ss->error_log, "Session[%d]: file %s open failed: %s",
                                   ss->conn_sock.GetFd(),
                                   upload_path.c_str(),
                                   err.message.c_str());
            file_fail = true;
        } else {
            file.Write(it->file_content);
            file.Close();
            file_urls += upload_url + "\n";
            debug(ss->error_log, "Session[%d]: file %s created",
                                   ss->conn_sock.GetFd(),
                                   upload_path.c_str());
        }
    }

    printf("Ya tvar 3\n");

    if (file_fail) {
        return ss->res_code = 500, __OnHttpError(ss);
    }

    ss->res_code = 201;
    ss->http_writer.Write(file_urls);
    return __OnHttpResponse(ss);
}

void  HttpServer::__HandleUploadRequest(SessionCtx* ss, const WebRoute& route) {
    info(ss->access_log, "Session[%d]: upload request", ss->conn_sock.GetFd());

    std::list<UploadReq> files;

    bool  boundary_exists;
    std::string  boundary_token =
            "--" + GetBoundary(ss->http_req.headers.Get("Content-Type"), &boundary_exists);

    if (boundary_exists) {
        Tokenizator tkz(ss->http_req.body);
        bool run = true;
        std::string chunk_token;
        for (;;) {
            chunk_token = tkz.NextS(boundary_token, &run);
            if (run == false)
                break;
            if (chunk_token == "--" || chunk_token == "--\n" || chunk_token == "--\r\n")
                break;
            HandleChunk(chunk_token, &files);
        }
        return __OnUploadEnd(ss, route, files);
    } else {
        debug(ss->error_log, "Session[%d]: multipart form data boundary isn't exist, returning error", ss->conn_sock.GetFd());
        return ss->res_code = 400, __OnHttpError(ss);
    }
}

}  // namespace Webserver
