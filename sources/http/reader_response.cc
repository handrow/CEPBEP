#include "http/reader.h"

namespace Http {

using namespace __CommonHelpers;
using namespace __CommonParsers;

ResponseReader::State
ResponseReader::STT_SkipEmptyLines(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    if (Buffer_[I_] == '\n') {
        I_ += 1;
    } else if (Buffer_[I_] == '\r') {
        nextState = STT_SKIP_CRLF_EMPTY_LINES;
        I_ += 1;
    } else {
        nextState = STT_BUFF_RES_LINE;
        __FlushParsedBuffer();
    }
    return nextState;
}

ResponseReader::State
ResponseReader::STT_SkipCrlfEmptyLines(bool*) {
    State nextState;

    if (Buffer_[I_] == '\n') {
        nextState = STT_SKIP_EMPTY_LINES;
        I_ += 1;
    } else {
        nextState = STT_BUFF_RES_LINE;
        __FlushParsedBuffer();
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_BuffResLine(bool*) {
    State nextState = STT_BUFF_RES_LINE;

    if (Buffer_[I_] == '\n') {
        nextState = STT_PARSE_RES_LINE;
    }
    I_ += 1;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_ParseResLine(bool* run) {
    State nextState = STT_BUFF_HEADERS;
    Error_ = ParseResponseLine(__GetParsedBuffer(),
                            &Result_.Version,
                            &Result_.Code,
                            &Result_.CodeMessage);
    __FlushParsedBuffer();
    if (Error_.IsError()) {
        nextState = STT_ERROR_OCCURED;
        *run = false;
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_BuffHeaders(bool*) {
    State nextState = STT_BUFF_HEADER_PAIR;

    if (Buffer_[I_] == '\n') {
        nextState = STT_PARSE_HEADERS;
        I_ += 1;
    } else if (Buffer_[I_] == '\r') {
        nextState = STT_BUFF_HEADERS;
        I_ += 1;
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_BuffHeaderPair(bool*) {
    State nextState = STT_BUFF_HEADER_PAIR;

    if (Buffer_[I_] == '\n') {
        nextState = STT_BUFF_HEADERS;
    }
    I_ += 1;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_ParseHeaders(bool* run) {
    State nextState;

    Error_ = ParseHeaders(__GetParsedBuffer(), &Result_.Headers);
    __FlushParsedBuffer();

    if (Error_.IsError()) {
        nextState = STT_ERROR_OCCURED;
        *run = false;
    } else if (Headers::IsChunkedEncoding(Result_.Headers)) {
        nextState = STT_BUFF_CHUNK_SIZE;
    } else if (Headers::GetContentLength(Result_.Headers) > 0) {
        nextState = STT_READ_BODY_CONTENT_LENGTH;
    } else {
        nextState = STT_HAVE_MESSAGE;
        *run = false;
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_BuffChunkSize(bool*) {
    State nextState = STT_BUFF_CHUNK_SIZE;

    if (Buffer_[I_] == '\n') {
        nextState = STT_PARSE_CHUNK_SIZE;
    }
    I_ += 1;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_ParseChunkSize(bool* run) {
    State nextState = STT_READ_CHUNK_DATA;
    Error_ = ParseChunkSize(__GetParsedBuffer(),
                          &ChunkSize_);
    __FlushParsedBuffer();

    if (Error_.IsError()) {
        nextState = STT_ERROR_OCCURED;
        *run = false;
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_ReadChunkData(bool* run) {
    State nextState = STT_READ_CHUNK_DATA;

    if (Buffer_.size() >= ChunkSize_) {
        nextState = STT_SKIP_CRLF_CHUNK_DATA;
        I_ = ChunkSize_;
        Result_.Body += __GetParsedBuffer();
        __FlushParsedBuffer();
    } else {
        *run = false;
    }

    return nextState;
}

ResponseReader::State
ResponseReader::STT_SkipCrlfChunkData(bool* run) {
    State nextState = STT_SKIP_CRLF_CHUNK_DATA;

    if (Buffer_.substr(0, 2) == "\r\n") {
        nextState = (ChunkSize_ == 0) ? STT_HAVE_MESSAGE
                                         : STT_BUFF_CHUNK_SIZE;
        I_ += 2;
        __FlushParsedBuffer();
    } else if (Buffer_.substr(0, 1) == "\n") {
        nextState = (ChunkSize_ == 0) ? STT_HAVE_MESSAGE
                                         : STT_BUFF_CHUNK_SIZE;
        I_ += 1;
        __FlushParsedBuffer();
    } else if (Buffer_.size() == 0 || Buffer_.substr(0, 1) == "\r") {
        *run = false;
    } else {
        nextState = STT_ERROR_OCCURED;
        Error_ = Error(HTTP_READER_NO_CHUNK_CRLF_END, "No linefeed at the end of the chunk-data");
        *run = false;
    }

    if (nextState == STT_HAVE_MESSAGE)
        *run = false;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_ReadBodyContentLength(bool* run) {
    const USize contentLength = Headers::GetContentLength(Result_.Headers);
    State nextState = STT_READ_BODY_CONTENT_LENGTH;

    if (Buffer_.size() >= contentLength) {
        nextState = STT_HAVE_MESSAGE;
        I_ = contentLength;
        Result_.Body = __GetParsedBuffer();
        __FlushParsedBuffer();
    }

    *run = false;

    return nextState;
}

ResponseReader::State
ResponseReader::STT_HaveMessage(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    __ClearResponse();
    Error_ = Error(0);
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
        if (I_ >= Buffer_.size() && !__IsMetaState(State_))
            break;
        switch (State_) {
            case STT_SKIP_EMPTY_LINES:          State_ = STT_SkipEmptyLines(&run); break;
            case STT_SKIP_CRLF_EMPTY_LINES:     State_ = STT_SkipCrlfEmptyLines(&run); break;
            case STT_BUFF_RES_LINE:             State_ = STT_BuffResLine(&run); break;
            case STT_PARSE_RES_LINE:            State_ = STT_ParseResLine(&run); break;
            case STT_BUFF_HEADERS:              State_ = STT_BuffHeaders(&run); break;
            case STT_BUFF_HEADER_PAIR:          State_ = STT_BuffHeaderPair(&run); break;
            case STT_PARSE_HEADERS:             State_ = STT_ParseHeaders(&run); break;
            case STT_READ_BODY_CONTENT_LENGTH:  State_ = STT_ReadBodyContentLength(&run); break;
            case STT_BUFF_CHUNK_SIZE:           State_ = STT_BuffChunkSize(&run); break;
            case STT_PARSE_CHUNK_SIZE:          State_ = STT_ParseChunkSize(&run); break;
            case STT_READ_CHUNK_DATA:           State_ = STT_ReadChunkData(&run); break;
            case STT_SKIP_CRLF_CHUNK_DATA:      State_ = STT_SkipCrlfChunkData(&run); break;
            case STT_HAVE_MESSAGE:              State_ = STT_HaveMessage(&run); break;
            case STT_ERROR_OCCURED:             State_ = STT_ErrorOccured(&run); break;
        }
    }
}

bool            ResponseReader::__IsMetaState(State stt) {
    return stt == STT_PARSE_RES_LINE  ||
           stt == STT_PARSE_HEADERS   ||
           stt == STT_HAVE_MESSAGE    ||
           stt == STT_ERROR_OCCURED   ||
           stt == STT_READ_CHUNK_DATA ||
           stt == STT_READ_BODY_CONTENT_LENGTH;
}

void            ResponseReader::__ClearResponse() {
    Result_ = Response();
}

void            ResponseReader::__FlushParsedBuffer() {
    Buffer_ = Buffer_.substr(I_);
    I_ = 0;
}

std::string     ResponseReader::__GetParsedBuffer() const {
    return Buffer_.substr(0, I_);
}

void            ResponseReader::Read(const std::string& bytes) {
    Buffer_ += bytes;
}

void            ResponseReader::Reset() {
    I_ = 0;
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

Response         ResponseReader::GetMessage() const {
    return Result_;
}

}  // namespace Http
