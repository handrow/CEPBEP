#include "http/reader.h"

namespace Http {

using namespace __CommonHelpers;
using namespace __CommonParsers;

RequestReader::State
RequestReader::STT_SkipEmptyLines(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    if (__buffer[__i] == '\n') {
        __i += 1;
    } else if (__buffer[__i] == '\r') {
        next_state = STT_SKIP_CRLF_EMPTY_LINES;
        __i += 1;
    } else {
        next_state = STT_BUFF_REQ_LINE;
        __FlushParsedBuffer();
    }
    return next_state;
}

RequestReader::State
RequestReader::STT_SkipCrlfEmptyLines(bool*) {
    State next_state;

    if (__buffer[__i] == '\n') {
        next_state = STT_SKIP_EMPTY_LINES;
        __i += 1;
    } else {
        next_state = STT_BUFF_REQ_LINE;
        __FlushParsedBuffer();
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_BuffReqLine(bool*) {
    State next_state = STT_BUFF_REQ_LINE;

    if (__buffer[__i] == '\n') {
        next_state = STT_PARSE_REQ_LINE;
    }
    __i += 1;

    return next_state;
}

RequestReader::State
RequestReader::STT_ParseReqLine(bool* run) {
    State next_state = STT_BUFF_HEADERS;
    __err = ParseRequestLine(__GetParsedBuffer(),
                            &__req_data.method,
                            &__req_data.uri,
                            &__req_data.version);
    __FlushParsedBuffer();
    if (__err.IsError()) {
        next_state = STT_ERROR_OCCURED;
        *run = false;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_BuffHeaders(bool*) {
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

RequestReader::State
RequestReader::STT_BuffHeaderPair(bool*) {
    State next_state = STT_BUFF_HEADER_PAIR;

    if (__buffer[__i] == '\n') {
        next_state = STT_BUFF_HEADERS;
    }
    __i += 1;

    return next_state;
}

RequestReader::State
RequestReader::STT_ParseHeaders(bool* run) {
    State next_state;

    __err = ParseHeaders(__GetParsedBuffer(), &__req_data.headers);
    __FlushParsedBuffer();

    if (__err.IsError()) {
        next_state = STT_ERROR_OCCURED;
        *run = false;
    } else if (Headers::IsChunkedEncoding(__req_data.headers)) {
        next_state = STT_BUFF_CHUNK_SIZE;
    } else if (__req_data.method == METHOD_POST &&
               Headers::GetContentLength(__req_data.headers) > 0
    ) {
        next_state = STT_READ_BODY_CONTENT_LENGTH;
    } else {
        next_state = STT_HAVE_MESSAGE;
        *run = false;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_BuffChunkSize(bool*) {
    State next_state = STT_BUFF_CHUNK_SIZE;

    if (__buffer[__i] == '\n') {
        next_state = STT_PARSE_CHUNK_SIZE;
    }
    __i += 1;

    return next_state;
}

RequestReader::State
RequestReader::STT_ParseChunkSize(bool* run) {
    State next_state = STT_READ_CHUNK_DATA;
    __err = ParseChunkSize(__GetParsedBuffer(),
                          &__chunk_size);
    __FlushParsedBuffer();

    if (__err.IsError()) {
        next_state = STT_ERROR_OCCURED;
        *run = false;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_ReadChunkData(bool* run) {
    State next_state = STT_READ_CHUNK_DATA;

    if (__buffer.size() >= __chunk_size) {
        next_state = STT_SKIP_CRLF_CHUNK_DATA;
        __i = __chunk_size;
        __req_data.body += __GetParsedBuffer();
        __FlushParsedBuffer();
    } else {
        *run = false;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_SkipCrlfChunkData(bool* run) {
    State next_state = STT_SKIP_CRLF_CHUNK_DATA;

    if (__buffer.substr(0, 2) == "\r\n") {
        next_state = (__chunk_size == 0) ? STT_HAVE_MESSAGE
                                         : STT_BUFF_CHUNK_SIZE;
        __i += 2;
        __FlushParsedBuffer();
    } else if (__buffer.substr(0, 1) == "\n") {
        next_state = (__chunk_size == 0) ? STT_HAVE_MESSAGE
                                         : STT_BUFF_CHUNK_SIZE;
        __i += 1;
        __FlushParsedBuffer();
    } else if (__buffer.size() == 0 || __buffer.substr(0, 1) == "\r") {
        *run = false;
    } else {
        next_state = STT_ERROR_OCCURED;
        __err = Error(HTTP_READER_NO_CHUNK_CRLF_END, "No linefeed at the end of the chunk-data");
        *run = false;
    }

    if (next_state == STT_HAVE_MESSAGE)
        *run = false;

    return next_state;
}


RequestReader::State
RequestReader::STT_ReadBodyContentLength(bool* run) {
    const usize content_len = Headers::GetContentLength(__req_data.headers);
    State next_state = STT_READ_BODY_CONTENT_LENGTH;

    if (__buffer.size() >= content_len) {
        next_state = STT_HAVE_MESSAGE;
        __i = content_len;
        __req_data.body = __GetParsedBuffer();
        __FlushParsedBuffer();
    }

    *run = false;

    return next_state;
}

RequestReader::State
RequestReader::STT_HaveMessage(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    __ClearRequest();
    __err = Error(0);
    return next_state;
}

RequestReader::State
RequestReader::STT_ErrorOccured(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    __ClearRequest();
    __err = Error(0);
    return next_state;
}

void            RequestReader::Process() {
    bool run = true;

    while (run) {
        if (__i >= __buffer.size() && !__IsMetaState(__state))
            break;
        switch (__state) {
            case STT_SKIP_EMPTY_LINES:          __state = STT_SkipEmptyLines(&run); break;
            case STT_SKIP_CRLF_EMPTY_LINES:     __state = STT_SkipCrlfEmptyLines(&run); break;
            case STT_BUFF_REQ_LINE:             __state = STT_BuffReqLine(&run); break;
            case STT_PARSE_REQ_LINE:            __state = STT_ParseReqLine(&run); break;
            case STT_BUFF_HEADERS:              __state = STT_BuffHeaders(&run); break;
            case STT_BUFF_HEADER_PAIR:          __state = STT_BuffHeaderPair(&run); break;
            case STT_PARSE_HEADERS:             __state = STT_ParseHeaders(&run); break;
            case STT_READ_BODY_CONTENT_LENGTH:  __state = STT_ReadBodyContentLength(&run); break;
            case STT_BUFF_CHUNK_SIZE:           __state = STT_BuffChunkSize(&run); break;
            case STT_PARSE_CHUNK_SIZE:          __state = STT_ParseChunkSize(&run); break;
            case STT_READ_CHUNK_DATA:           __state = STT_ReadChunkData(&run); break;
            case STT_SKIP_CRLF_CHUNK_DATA:      __state = STT_SkipCrlfChunkData(&run); break;
            case STT_HAVE_MESSAGE:              __state = STT_HaveMessage(&run); break;
            case STT_ERROR_OCCURED:             __state = STT_ErrorOccured(&run); break;
        }
    }
}

bool            RequestReader::__IsMetaState(State stt) {
    return stt == STT_PARSE_REQ_LINE  ||
           stt == STT_PARSE_HEADERS   ||
           stt == STT_HAVE_MESSAGE    ||
           stt == STT_READ_CHUNK_DATA ||
           stt == STT_ERROR_OCCURED;
}

void            RequestReader::__ClearRequest() {
    __req_data = Request();
}

void            RequestReader::__FlushParsedBuffer() {
    __buffer = __buffer.substr(__i);
    __i = 0;
}

std::string     RequestReader::__GetParsedBuffer() const {
    return __buffer.substr(0, __i);
}

void            RequestReader::Read(const std::string& bytes) {
    __buffer += bytes;
}

void            RequestReader::Reset() {
    __i = 0;
    __err = Error(0);
    __state = STT_SKIP_EMPTY_LINES;
    __buffer.clear();
    __ClearRequest();
}

bool            RequestReader::HasMessage() const {
    return __state == STT_HAVE_MESSAGE;
}

bool            RequestReader::HasError() const {
    return __state == STT_ERROR_OCCURED;
}

Error           RequestReader::GetError() const {
    return __err;
}

Request         RequestReader::GetMessage() const {
    return __req_data;
}

}  // namespace Http
