#include "http/reader.h"

namespace Http {

using namespace __CommonHelpers;
using namespace __CommonParsers;

RequestReader::State
RequestReader::STT_SkipEmptyLines(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    if (Buffer_[I_] == '\n') {
        I_ += 1;
    } else if (Buffer_[I_] == '\r') {
        nextState = STT_SKIP_CRLF_EMPTY_LINES;
        I_ += 1;
    } else {
        nextState = STT_BUFF_META;
        __FlushParsedBuffer();
    }
    return nextState;
}

RequestReader::State
RequestReader::STT_SkipCrlfEmptyLines(bool*) {
    State nextState;

    if (Buffer_[I_] == '\n') {
        nextState = STT_SKIP_EMPTY_LINES;
        I_ += 1;
    } else {
        nextState = STT_BUFF_META;
        __FlushParsedBuffer();
    }

    return nextState;
}

RequestReader::State
RequestReader::STT_ParseMeta(bool* run) {
    std::string metaBuffer = __GetParsedBuffer();
    __FlushParsedBuffer();

    USize delim = metaBuffer.find_first_of("\n") + 1;

    std::string  requestLineBuffer = metaBuffer.substr(0, delim);
    std::string  headersBuff = metaBuffer.substr(delim);

    Error_ = ParseRequestLine(requestLineBuffer,
                            &Result_.Method,
                            &Result_.Uri,
                            &Result_.Version);
    if (Error_.IsError())
        return *run = false, STT_ERROR_OCCURED;

    Error_ = ParseHeaders(headersBuff, &Result_.Headers);

    if (Error_.IsError()) {
        return *run = false, STT_ERROR_OCCURED;
    } else if (Result_.Method != METHOD_GET && Headers::IsChunkedEncoding(Result_.Headers)) {
        return STT_BUFF_CHUNK_SIZE;
    } else if (Result_.Method != METHOD_GET && Headers::GetContentLength(Result_.Headers) > 0) {
        return STT_READ_BODY_CONTENT_LENGTH;
    }

    return *run = false, STT_HAVE_MESSAGE;
}

RequestReader::State
RequestReader::STT_BuffMeta(bool*) {
    State nextState = STT_BUFF_META_LINE;

    if (Buffer_[I_] == '\n') {
        nextState = STT_PARSE_META;
        I_ += 1;
    } else if (Buffer_[I_] == '\r') {
        nextState = STT_BUFF_META;
        I_ += 1;
    }

    return nextState;
}

RequestReader::State
RequestReader::STT_BuffMetaLine(bool*) {
    State nextState = STT_BUFF_META_LINE;

    if (Buffer_[I_] == '\n') {
        nextState = STT_BUFF_META;
    }
    I_ += 1;

    return nextState;
}

RequestReader::State
RequestReader::STT_BuffChunkSize(bool*) {
    State nextState = STT_BUFF_CHUNK_SIZE;

    if (Buffer_[I_] == '\n') {
        nextState = STT_PARSE_CHUNK_SIZE;
    }
    I_ += 1;

    return nextState;
}

RequestReader::State
RequestReader::STT_ParseChunkSize(bool* run) {
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

RequestReader::State
RequestReader::STT_ReadChunkData(bool* run) {
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

RequestReader::State
RequestReader::STT_SkipCrlfChunkData(bool* run) {
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


RequestReader::State
RequestReader::STT_ReadBodyContentLength(bool* run) {
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

RequestReader::State
RequestReader::STT_HaveMessage(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    __ClearRequest();
    Error_ = Error(0);
    return nextState;
}

RequestReader::State
RequestReader::STT_ErrorOccured(bool*) {
    State nextState = STT_SKIP_EMPTY_LINES;

    __ClearRequest();
    Error_ = Error(0);
    return nextState;
}

void            RequestReader::Process() {
    bool run = true;

    while (run) {
        if (I_ >= Buffer_.size() && !__IsMetaState(State_))
            break;
        switch (State_) {
            case STT_SKIP_EMPTY_LINES:          State_ = STT_SkipEmptyLines(&run); break;
            case STT_SKIP_CRLF_EMPTY_LINES:     State_ = STT_SkipCrlfEmptyLines(&run); break;
            case STT_PARSE_META:                State_ = STT_ParseMeta(&run); break;
            case STT_BUFF_META:                 State_ = STT_BuffMeta(&run); break;
            case STT_BUFF_META_LINE:            State_ = STT_BuffMetaLine(&run); break;
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

bool            RequestReader::__IsMetaState(State stt) {
    return stt == STT_PARSE_META      ||
           stt == STT_HAVE_MESSAGE    ||
           stt == STT_READ_CHUNK_DATA ||
           stt == STT_ERROR_OCCURED;
}

void            RequestReader::__ClearRequest() {
    Result_ = Request();
}

void            RequestReader::__FlushParsedBuffer() {
    Buffer_ = Buffer_.substr(I_);
    I_ = 0;
}

std::string     RequestReader::__GetParsedBuffer() const {
    return Buffer_.substr(0, I_);
}

void            RequestReader::Read(const std::string& bytes) {
    Buffer_ += bytes;
}

void            RequestReader::Reset() {
    I_ = 0;
    Error_ = Error(0);
    State_ = STT_SKIP_EMPTY_LINES;
    Buffer_.clear();
    __ClearRequest();
}

bool            RequestReader::HasMessage() const {
    return State_ == STT_HAVE_MESSAGE;
}

bool            RequestReader::HasError() const {
    return State_ == STT_ERROR_OCCURED;
}

Error           RequestReader::GetError() const {
    return Error_;
}

Request         RequestReader::GetMessage() const {
    return Result_;
}

}  // namespace Http
