#include "cgi/response_reader.h"

namespace Cgi {

ResponseReader::State
ResponseReader::STT_SkipEmptyLines(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    if (Buffer_[Idx_] == '\n') {
        Idx_ += 1;
    } else if (Buffer_[Idx_] == '\r') {
        nextState = STT_SKIP_CRLF_EMPTY_LINES;
        Idx_ += 1;
    } else {
        nextState = STT_BUFF_HEADERS;
        __FlushParsedBuffer();
    }
    return nextState;
}

ResponseReader::State
ResponseReader::STT_SkipCrlfEmptyLines(bool*) {
    State nextState;

    if (Buffer_[Idx_] == '\n') {
        nextState = STT_SKIP_EMPTY_LINES;
        Idx_ += 1;
    } else {
        nextState = STT_BUFF_HEADERS;
        __FlushParsedBuffer();
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_BuffHeaders(bool*) {
    State nextState = STT_BUFF_HEADER_PAIR;

    if (Buffer_[Idx_] == '\n') {
        nextState = STT_PARSE_HEADERS;
        Idx_ += 1;
    } else if (Buffer_[Idx_] == '\r') {
        nextState = STT_BUFF_HEADERS;
        Idx_ += 1;
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_BuffHeaderPair(bool*) {
    State nextState = STT_BUFF_HEADER_PAIR;

    if (Buffer_[Idx_] == '\n') {
        nextState = STT_BUFF_HEADERS;
    }
    Idx_ += 1;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_ParseHeaders(bool* run) {
    State nextState;

    Error_ = Http::__CommonParsers::ParseHeaders(__GetParsedBuffer(), &Result_.Headers);
    __FlushParsedBuffer();

    if (Error_.IsError()) {
        nextState = STT_ERROR_OCCURED;
        *run = false;
    } else if (Http::Headers::GetContentLength(Result_.Headers) > 0) {
        nextState = STT_READ_BODY_CONTENT_LENGTH;
    } else {
        nextState = STT_READ_BODY_INFINITE;
    }

    return nextState;
}

void ResponseReader::__SetStatus() {
    Error_ = Http::__CommonParsers::ParseCgiStatus(Result_.Headers,
                                                &Result_.Version,
                                                &Result_.Code,
                                                &Result_.CodeMessage);
    Result_.Headers.Rm("Status");
}

ResponseReader::State
ResponseReader::STT_ReadBodyContentLength(bool* run) {
    const USize contentLength = Http::Headers::GetContentLength(Result_.Headers);
    State nextState = STT_READ_BODY_CONTENT_LENGTH;

    if (Buffer_.size() >= contentLength) {
        nextState = STT_HAVE_MESSAGE;
        Idx_ = contentLength;
        Result_.Body = __GetParsedBuffer();
        __FlushParsedBuffer();
        __SetStatus();
        if (Error_.IsError())
            nextState = STT_ERROR_OCCURED;
    }

    *run = false;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_ReadBodyInfinite(bool* run) {
    State nextState = STT_READ_BODY_INFINITE;

    if (Buffer_.size() >= EndOfRead_) {
        nextState = STT_HAVE_MESSAGE;
        Idx_ = EndOfRead_;
        Result_.Body = __GetParsedBuffer();
        __FlushParsedBuffer();

        __SetStatus();
        Result_.Headers.Set("Content-Length",
                               Convert<std::string>(Result_.Body.size()));
    }

    *run = false;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_HaveMessage(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    __ClearResponse();
    return nextState;
}

ResponseReader::State
ResponseReader::STT_ErrorOccured(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    __ClearResponse();
    Error_ = Error(0);
    return nextState;
}

void            ResponseReader::Process() {
    bool run = true;

    while (run) {
        if (Idx_ >= Buffer_.size() && !__IsMetaState(State_))
            break;
        switch (State_) {
            case STT_SKIP_EMPTY_LINES:          State_ = STT_SkipEmptyLines(&run); break;
            case STT_SKIP_CRLF_EMPTY_LINES:     State_ = STT_SkipCrlfEmptyLines(&run); break;
            case STT_BUFF_HEADERS:              State_ = STT_BuffHeaders(&run); break;
            case STT_BUFF_HEADER_PAIR:          State_ = STT_BuffHeaderPair(&run); break;
            case STT_PARSE_HEADERS:             State_ = STT_ParseHeaders(&run); break;
            case STT_READ_BODY_CONTENT_LENGTH:  State_ = STT_ReadBodyContentLength(&run); break;
            case STT_READ_BODY_INFINITE:        State_ = STT_ReadBodyInfinite(&run); break;
            case STT_HAVE_MESSAGE:              State_ = STT_HaveMessage(&run); break;
            case STT_ERROR_OCCURED:             State_ = STT_ErrorOccured(&run); break;
        }
    }
}

void            ResponseReader::EndRead() {
    EndOfRead_ = Buffer_.size();
}

void            ResponseReader::Read(const std::string& bytes) {
    Buffer_ += bytes;
}

bool            ResponseReader::__IsMetaState(State stt) {
    return stt == STT_PARSE_HEADERS             ||
           stt == STT_HAVE_MESSAGE              ||
           stt == STT_ERROR_OCCURED             ||
           stt == STT_READ_BODY_INFINITE        ||
           stt == STT_READ_BODY_CONTENT_LENGTH;
}

void            ResponseReader::__ClearResponse() {
    Result_ = Http::Response();
}

void            ResponseReader::__FlushParsedBuffer() {
    Buffer_ = Buffer_.substr(Idx_);
    Idx_ = 0;
}

std::string     ResponseReader::__GetParsedBuffer() const {
    return Buffer_.substr(0, Idx_);
}

void            ResponseReader::Reset() {
    Idx_ = 0;
    EndOfRead_ = std::string::npos;
    Error_ = Error(0);
    State_ = STT_SKIP_EMPTY_LINES;
    Buffer_.clear();
    __ClearResponse();
}

bool            ResponseReader::HasMessage() const {
    return State_ == STT_HAVE_MESSAGE;
}

bool            ResponseReader::HasError() const {
    return State_ == STT_ERROR_OCCURED;
}

Error           ResponseReader::GetError() const {
    return Error_;
}

Http::Response  ResponseReader::GetMessage() const {
    return Result_;
}

}  // namespace Cgi
