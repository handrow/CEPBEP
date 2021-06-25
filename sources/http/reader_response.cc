#include "http/reader.h"

namespace Http {

using namespace __CommonHelpers;
using namespace __CommonParsers;

ResponseReader::State
ResponseReader::STT_SkipEmptyLines(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    if (__buffer[__i] == '\n') {
        __i += 1;
    } else if (__buffer[__i] == '\r') {
        next_state = STT_SKIP_CRLF_EMPTY_LINES;
        __i += 1;
    } else {
        next_state = STT_BUFF_RES_LINE;
        __FlushParsedBuffer();
    }
    return next_state;
}

ResponseReader::State
ResponseReader::STT_SkipCrlfEmptyLines(bool*) {
    State next_state;

    if (__buffer[__i] == '\n') {
        next_state = STT_SKIP_EMPTY_LINES;
        __i += 1;
    } else {
        next_state = STT_BUFF_RES_LINE;
        __FlushParsedBuffer();
    }

    return next_state;
}

ResponseReader::State
ResponseReader::STT_BuffResLine(bool*) {
    State next_state = STT_BUFF_RES_LINE;

    if (__buffer[__i] == '\n') {
        next_state = STT_PARSE_RES_LINE;
    }
    __i += 1;

    return next_state;
}

ResponseReader::State
ResponseReader::STT_ParseResLine(bool* run) {
    State next_state = STT_BUFF_HEADERS;
    __err = ParseResponseLine(__GetParsedBuffer(),
                            &__res_data.version,
                            &__res_data.code,
                            &__res_data.code_message);
    __FlushParsedBuffer();
    if (__err.IsError()) {
        next_state = STT_ERROR_OCCURED;
        *run = false;
    }

    return next_state;
}

ResponseReader::State
ResponseReader::STT_BuffHeaders(bool*) {
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

ResponseReader::State
ResponseReader::STT_BuffHeaderPair(bool*) {
    State next_state = STT_BUFF_HEADER_PAIR;

    if (__buffer[__i] == '\n') {
        next_state = STT_BUFF_HEADERS;
    }
    __i += 1;

    return next_state;
}

ResponseReader::State
ResponseReader::STT_ParseHeaders(bool* run) {
    State next_state;

    __err = ParseHeaders(__GetParsedBuffer(), &__res_data.headers);
    __FlushParsedBuffer();

    if (__err.IsError()) {
        next_state = STT_ERROR_OCCURED;
        *run = false;
    } else if (Headers::GetContentLength(__res_data.headers) > 0) {
        next_state = STT_READ_BODY_CONTENT_LENGTH;
    } else {
        next_state = STT_HAVE_MESSAGE;
        *run = false;
    }

    return next_state;
}

ResponseReader::State
ResponseReader::STT_ReadBodyContentLength(bool* run) {
    const usize content_len = Headers::GetContentLength(__res_data.headers);
    State next_state = STT_READ_BODY_CONTENT_LENGTH;

    if (__buffer.size() >= content_len) {
        next_state = STT_HAVE_MESSAGE;
        __i = content_len;
        __res_data.body = __GetParsedBuffer();
        __FlushParsedBuffer();
    }

    *run = false;

    return next_state;
}

ResponseReader::State
ResponseReader::STT_HaveMessage(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    __ClearResponse();
    __err = Error(0);
    return next_state;
}

ResponseReader::State
ResponseReader::STT_ErrorOccured(bool*) {
    State next_state = STT_SKIP_EMPTY_LINES;

    __ClearResponse();
    __err = Error(0);
    return next_state;
}

void            ResponseReader::Process() {
    bool run = true;

    while (run) {
        if (__i >= __buffer.size() && !__IsMetaState(__state))
            break;
        switch (__state) {
            case STT_SKIP_EMPTY_LINES:          __state = STT_SkipEmptyLines(&run); break;
            case STT_SKIP_CRLF_EMPTY_LINES:     __state = STT_SkipCrlfEmptyLines(&run); break;
            case STT_BUFF_RES_LINE:             __state = STT_BuffResLine(&run); break;
            case STT_PARSE_RES_LINE:            __state = STT_ParseResLine(&run); break;
            case STT_BUFF_HEADERS:              __state = STT_BuffHeaders(&run); break;
            case STT_BUFF_HEADER_PAIR:          __state = STT_BuffHeaderPair(&run); break;
            case STT_PARSE_HEADERS:             __state = STT_ParseHeaders(&run); break;
            case STT_READ_BODY_CONTENT_LENGTH:  __state = STT_ReadBodyContentLength(&run); break;
            case STT_HAVE_MESSAGE:              __state = STT_HaveMessage(&run); break;
            case STT_ERROR_OCCURED:             __state = STT_ErrorOccured(&run); break;
        }
    }
}

bool            ResponseReader::__IsMetaState(State stt) {
    return stt == STT_PARSE_RES_LINE ||
           stt == STT_PARSE_HEADERS  ||
           stt == STT_HAVE_MESSAGE   ||
           stt == STT_ERROR_OCCURED  ||
           stt == STT_READ_BODY_CONTENT_LENGTH;
}

void            ResponseReader::__ClearResponse() {
    __res_data = Response();
}

void            ResponseReader::__FlushParsedBuffer() {
    __buffer = __buffer.substr(__i);
    __i = 0;
}

std::string     ResponseReader::__GetParsedBuffer() const {
    return __buffer.substr(0, __i);
}

void            ResponseReader::Read(const std::string& bytes) {
    __buffer += bytes;
}

void            ResponseReader::Reset() {
    __i = 0;
    __err = Error(0);
    __state = STT_SKIP_EMPTY_LINES;
    __buffer.clear();
    __ClearResponse();
}

bool            ResponseReader::HasMessage() const {
    return __state == STT_HAVE_MESSAGE;
}

bool            ResponseReader::HasError() const {
    return __state == STT_ERROR_OCCURED;
}

Error           ResponseReader::GetError() const {
    return __err;
}

Response         ResponseReader::GetMessage() const {
    return __res_data;
}
}  // namespace Http
