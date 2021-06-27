#ifndef HTTP_READER_H_
#define HTTP_READER_H_

#include "http/http.h"
#include "common/error.h"

namespace Http {

enum  ReaderErrorCodes {
    HTTP_READER_ERROR = 30000,
    HTTP_READER_BAD_START_LINE,
    HTTP_READER_BAD_METHOD,
    HTTP_READER_BAD_PROTOCOL_VERSION,
    HTTP_READER_BAD_TOKEN_IN_REQLINE,
    HTTP_READER_BAD_HEADER_KEY,
    HTTP_READER_BAD_HEADER_VALUE,
    HTTP_READER_BAD_CODE,
    HTTP_READER_NO_CHUNK_CRLF_END
};


namespace __CommonHelpers {

usize       IsWhiteSpace(const std::string& s, usize i);
bool        IsSeparator(char sym);
bool        IsValidHdrKey(char sym);
bool        IsValidHdrValue(char sym);

}

namespace __CommonParsers {

// VALID: *(WS) (VERSION) *(WS) (CODE) *(WS) (PHRASE) (CRLF)
Error  ParseResponseLine(const std::string& buff, ProtocolVersion* ver, int* rcode, std::string* phrase);

// VALID: *(WS) (METHOD) *(WS) (URI) *(WS) (VERSION) *(WS) (CRLF)
Error  ParseRequestLine(const std::string& buff, Method* mtd, URI* uri, ProtocolVersion* ver);

// VALID: *(HDR_PAIR) (CRLF)
// HDR_PAIR: *(WS) (HDR_KEY) (":") *(WS) (HDR_VALUE) *(WS) (CRLF)
Error  ParseHeaders(const std::string& buff, Headers* hdrs);

// VALID: *(HEX) (CRLF)
Error  ParseChunkSize(const std::string& buff, usize* chunk_size);

}  // namespace __CommonParsers

class RequestReader {
 private:
    enum   State {
        STT_SKIP_EMPTY_LINES,  // input needed
        STT_SKIP_CRLF_EMPTY_LINES,  // input needed

        STT_BUFF_REQ_LINE,  // input needed
        STT_PARSE_REQ_LINE,

        STT_BUFF_HEADERS,  // input needed
        STT_BUFF_HEADER_PAIR,  // input needed
        STT_PARSE_HEADERS,

        STT_READ_BODY_CONTENT_LENGTH,  // input needed
        STT_BUFF_CHUNK_SIZE,           // input needed
        STT_PARSE_CHUNK_SIZE,
        STT_READ_CHUNK_DATA,
        STT_SKIP_CRLF_CHUNK_DATA,      // input needed

        STT_HAVE_MESSAGE,  // makes self-pause
        STT_ERROR_OCCURED,  // makes self-pause
    };

    usize           __i;
    Error           __err;
    State           __state;
    Request         __req_data;
    usize           __chunk_size;
    std::string     __buffer;

 private:
    static bool     __IsMetaState(State stt);

    void            __ClearRequest();
    void            __FlushParsedBuffer();
    std::string     __GetParsedBuffer() const;

    State   STT_SkipEmptyLines(bool* run);
    State   STT_SkipCrlfEmptyLines(bool* run);
    State   STT_BuffReqLine(bool* run);
    State   STT_ParseReqLine(bool* run);
    State   STT_BuffHeaders(bool* run);
    State   STT_BuffHeaderPair(bool* run);
    State   STT_ParseHeaders(bool* run);
    State   STT_ReadBodyContentLength(bool* run);
    State   STT_BuffChunkSize(bool* run);
    State   STT_ParseChunkSize(bool* run);
    State   STT_ReadChunkData(bool* run);
    State   STT_SkipCrlfChunkData(bool* run);
    State   STT_HaveMessage(bool* run);
    State   STT_ErrorOccured(bool* run);

 public:
    RequestReader() { Reset(); }

    void    Read(const std::string& bytes);
    void    Process();
    void    Reset();

    bool    HasMessage() const;
    bool    HasError() const;

    Error   GetError() const;
    Request GetMessage() const;
};

class ResponseReader {
 private:
    enum   State {
        STT_SKIP_EMPTY_LINES,  // input needed
        STT_SKIP_CRLF_EMPTY_LINES,  // input needed

        STT_BUFF_RES_LINE,  // input needed
        STT_PARSE_RES_LINE,

        STT_BUFF_HEADERS,  // input needed
        STT_BUFF_HEADER_PAIR,  // input needed
        STT_PARSE_HEADERS,

        STT_READ_BODY_CONTENT_LENGTH,  // input needed, but meta
        STT_BUFF_CHUNK_SIZE,           // input needed
        STT_PARSE_CHUNK_SIZE,
        STT_READ_CHUNK_DATA,
        STT_SKIP_CRLF_CHUNK_DATA,      // input needed

        STT_HAVE_MESSAGE,  // makes self-pause
        STT_ERROR_OCCURED,  // makes self-pause
    };

    usize           __i;
    Error           __err;
    State           __state;
    Response        __res_data;
    usize           __chunk_size;
    std::string     __buffer;

 private:
    static bool     __IsMetaState(State stt);

    void            __ClearResponse();
    void            __FlushParsedBuffer();
    std::string     __GetParsedBuffer() const;

    State   STT_SkipEmptyLines(bool* run);
    State   STT_SkipCrlfEmptyLines(bool* run);
    State   STT_BuffResLine(bool* run);
    State   STT_ParseResLine(bool* run);
    State   STT_BuffHeaders(bool* run);
    State   STT_BuffHeaderPair(bool* run);
    State   STT_ParseHeaders(bool* run);
    State   STT_ReadBodyContentLength(bool* run);
    State   STT_BuffChunkSize(bool* run);
    State   STT_ParseChunkSize(bool* run);
    State   STT_ReadChunkData(bool* run);
    State   STT_SkipCrlfChunkData(bool* run);
    State   STT_HaveMessage(bool* run);
    State   STT_ErrorOccured(bool* run);

 public:
    ResponseReader() { Reset(); }

    void    Read(const std::string& bytes);
    void    Process();
    void    Reset();

    bool    HasMessage() const;
    bool    HasError() const;

    Error    GetError() const;
    Response GetMessage() const;
};

}  // namespace Http

#endif  // HTTP_READER_H_
