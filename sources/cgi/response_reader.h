#ifndef CGI_RESPONSE_READER_H_
#define CGI_RESPONSE_READER_H_

#include "http/reader.h"

namespace Cgi {

class ResponseReader {
 private:
    enum   State {
        STT_SKIP_EMPTY_LINES,  // input needed
        STT_SKIP_CRLF_EMPTY_LINES,  // input needed

        STT_BUFF_HEADERS,  // input needed
        STT_BUFF_HEADER_PAIR,  // input needed
        STT_PARSE_HEADERS,

        STT_READ_BODY_CONTENT_LENGTH,  // input needed, but meta
        STT_READ_BODY_INFINITE,

        STT_HAVE_MESSAGE,  // makes self-pause
        STT_ERROR_OCCURED,  // makes self-pause
    };

    USize           Idx_;
    Error           Error_;
    State           State_;
    Http::Response  Result_;
    USize           EndOfRead_;
    std::string     Buffer_;


 private:
    static bool     IsMetaState(State stt);

    void            SetStatus();
    void            ClearResponse();
    void            FlushParsedBuffer();
    std::string     GetParsedBuffer() const;

    State   STT_SkipEmptyLines(bool* run);
    State   STT_SkipCrlfEmptyLines(bool* run);
    State   STT_BuffHeaders(bool* run);
    State   STT_BuffHeaderPair(bool* run);
    State   STT_ParseHeaders(bool* run);
    State   STT_ReadBodyContentLength(bool* run);
    State   STT_ReadBodyInfinite(bool *run);
    State   STT_HaveMessage(bool* run);
    State   STT_ErrorOccured(bool* run);

 public:
    ResponseReader() { Reset(); }

    void    Read(const std::string& bytes);
    void    EndRead();

    void    Process();
    void    Reset();

    bool    HasMessage() const;
    bool    HasError() const;

    Error          GetError() const;
    Http::Response GetMessage() const;
};

}  // namespace Cgi

#endif  // CGI_RESPONSE_READER_H_
