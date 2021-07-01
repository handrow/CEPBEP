#include "cgi_response_reader.h"

namespace Http {

CgiResponseReader::State
CgiResponseReader::STT_SkipEmptyLines(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    if (__buffer[__i] == '\n') {
        __i += 1;
    } else if (__buffer[__i] == '\r') {
        next_state = STT_SKIP_CRLF_EMPTY_LINES;
        __i += 1;
    } else {
        next_state = STT_BUFF_HEADERS;
        __FlushParsedBuffer();
    }
    return next_state;
}

CgiResponseReader::State
CgiResponseReader::STT_SkipCrlfEmptyLines(bool*) {
    State next_state;

    if (__buffer[__i] == '\n') {
        next_state = STT_SKIP_EMPTY_LINES;
        __i += 1;
    } else {
        next_state = STT_BUFF_HEADERS;
        __FlushParsedBuffer();
    }

    return next_state;
}

CgiResponseReader::State
CgiResponseReader::STT_BuffHeaders(bool*) {
    State next_state = STT_BUFF_HEADER_PAIR;

    if (__buffer[__i] == '\n') {
        next_state = STT_PARSE_HEADERS;
        __i += 1;
    } else if (__buffer[__i] == '\r') {
        next_state = STT_BUFF_HEADERS;
        __i += 1;
    }

    return next_state;
}

CgiResponseReader::State
CgiResponseReader::STT_BuffHeaderPair(bool*) {
    State next_state = STT_BUFF_HEADER_PAIR;

    if (__buffer[__i] == '\n') {
        next_state = STT_BUFF_HEADERS;
    }
    __i += 1;

    return next_state;
}

CgiResponseReader::State
CgiResponseReader::STT_ParseHeaders(bool* run) {
    State next_state;

    __err = __CommonParsers::ParseHeaders(__GetParsedBuffer(), &__res_data.headers);
    __FlushParsedBuffer();

    if (__err.IsError()) {
        next_state = STT_ERROR_OCCURED;
        *run = false;
    } else if (Headers::GetContentLength(__res_data.headers) > 0) {
        next_state = STT_READ_BODY_CONTENT_LENGTH;
    } else {
        next_state = STT_READ_BODY_INFINITE;
    }

    return next_state;
}

void CgiResponseReader::__SetStatus() {
    State next_state;
    __err = __CommonParsers::ParseCgiStatus(__res_data.headers,
                                            &__res_data.version,
                                            &__res_data.code,
                                            &__res_data.code_message);
    if (__err.IsError())
        next_state = STT_ERROR_OCCURED;

    __res_data.headers.Rm("Status");
}

CgiResponseReader::State
CgiResponseReader::STT_ReadBodyContentLength(bool* run) {
    const usize content_len = Headers::GetContentLength(__res_data.headers);
    State next_state = STT_READ_BODY_CONTENT_LENGTH;

    if (__buffer.size() >= content_len) {
        next_state = STT_HAVE_MESSAGE;
        __i = content_len;
        __res_data.body = __GetParsedBuffer();
        __FlushParsedBuffer();
        __SetStatus();
    }

    *run = false;

    return next_state;
}

CgiResponseReader::State
CgiResponseReader::STT_ReadBodyInfinite(bool* run) {
    State next_state = STT_READ_BODY_INFINITE;

    if (__buffer.size() >= __end_read_i) {
        next_state = STT_HAVE_MESSAGE;
        __i = __end_read_i;
        __res_data.body = __GetParsedBuffer();
        __FlushParsedBuffer();

        __SetStatus();
        __res_data.headers.Set("Content-Length",
                               Convert<std::string>(__res_data.body.size()));
    }

    *run = false;

    return next_state;
}

CgiResponseReader::State
CgiResponseReader::STT_HaveMessage(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    __ClearResponse();
    return next_state;
}

CgiResponseReader::State
CgiResponseReader::STT_ErrorOccured(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    __ClearResponse();
    __err = Error(0);
    return next_state;
}

void            CgiResponseReader::Process() {
    bool run = true;

    while (run) {
        if (__i >= __buffer.size() && !__IsMetaState(__state))
            break;
        switch (__state) {
            case STT_SKIP_EMPTY_LINES:          __state = STT_SkipEmptyLines(&run); break;
            case STT_SKIP_CRLF_EMPTY_LINES:     __state = STT_SkipCrlfEmptyLines(&run); break;
            case STT_BUFF_HEADERS:              __state = STT_BuffHeaders(&run); break;
            case STT_BUFF_HEADER_PAIR:          __state = STT_BuffHeaderPair(&run); break;
            case STT_PARSE_HEADERS:             __state = STT_ParseHeaders(&run); break;
            case STT_READ_BODY_CONTENT_LENGTH:  __state = STT_ReadBodyContentLength(&run); break;
            case STT_READ_BODY_INFINITE:        __state = STT_ReadBodyInfinite(&run); break;
            case STT_HAVE_MESSAGE:              __state = STT_HaveMessage(&run); break;
            case STT_ERROR_OCCURED:             __state = STT_ErrorOccured(&run); break;
        }
    }
}

void            CgiResponseReader::EndRead() {
    __end_read_i = __buffer.size();
}

void            CgiResponseReader::Read(const std::string& bytes) {
    __buffer += bytes;
}

bool            CgiResponseReader::__IsMetaState(State stt) {
    return stt == STT_PARSE_HEADERS             ||
           stt == STT_HAVE_MESSAGE              ||
           stt == STT_ERROR_OCCURED             ||
           stt == STT_READ_BODY_INFINITE        ||
           stt == STT_READ_BODY_CONTENT_LENGTH;
}

void            CgiResponseReader::__ClearResponse() {
    __res_data = Response();
}

void            CgiResponseReader::__FlushParsedBuffer() {
    __buffer = __buffer.substr(__i);
    __i = 0;
}

std::string     CgiResponseReader::__GetParsedBuffer() const {
    return __buffer.substr(0, __i);
}

void            CgiResponseReader::Reset() {
    __i = 0;
    __end_read_i = std::string::npos;
    __err = Error(0);
    __state = STT_SKIP_EMPTY_LINES;
    __buffer.clear();
    __ClearResponse();
}

bool            CgiResponseReader::HasMessage() const {
    return __state == STT_HAVE_MESSAGE;
}

bool            CgiResponseReader::HasError() const {
    return __state == STT_ERROR_OCCURED;
}

Error           CgiResponseReader::GetError() const {
    return __err;
}

Response        CgiResponseReader::GetMessage() const {
    return __res_data;
}

}  // namespace Http
