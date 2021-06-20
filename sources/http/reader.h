#ifndef HTTP_READER_H_
#define HTTP_READER_H_

#include "http/http.h"
#include "common/error.h"

namespace Http {

enum  ReaderErrorCodes {
    HTTP_READER_NO_ERROR = 30000,
    HTTP_READER_BAD_METHOD,
    HTTP_READER_BAD_PROTOCOL_VERSION,
    HTTP_READER_BAD_TOKEN_IN_REQLINE,
    HTTP_READER_BAD_HEADER_KEY,
    HTTP_READER_BAD_HEADER_VALUE,
};

class RequestReader {
 private:
    enum   State {
        STT_INIT,
        STT_METHOD,
        STT_METHOD_END,
        STT_URI,
        STT_URI_END,
        STT_VER,
        STT_VER_END,
        STT_HDR_BEGIN,
        STT_HDR_K,
        STT_HDR_K_END,
        STT_HDR_V,
        STT_HDR_V_END,
        STT_META_END,
        STT_BODY_CL,
        STT_ERR,
        STT_END
    };

    Error           __err;
    State           __state;
    std::string     __remainder;
    usize           __i;

    std::string     __hdr_key_tmp;
    std::string     __hdr_val_tmp;
    Request         __req;

 private:
    void        __Parse();

    void        __FlushParsedData();
    std::string __GetParsedData() const;

    static bool __IsMetaState(State);

    State       STT_INIT_Handler(bool* run);
    State       STT_METHOD_Handler(bool* run);
    State       STT_METHOD_END_Handler(bool* run);
    State       STT_URI_Handler(bool* run);
    State       STT_URI_END_Handler(bool* run);
    State       STT_VER_Handler(bool* run);
    State       STT_VER_END_Handler(bool* run);
    State       STT_HDR_BEGIN_Handler(bool* run);
    State       STT_HDR_K_Handler(bool* run);
    State       STT_HDR_K_END_Handler(bool* run);
    State       STT_HDR_V_Handler(bool* run);
    State       STT_HDR_V_END_Handler(bool* run);
    State       STT_META_END_Handler(bool* run);
    State       STT_BODY_CL_Handler(bool* run);
    State       STT_ERR_Handler(bool* run);
    State       STT_END_Handler(bool* run);

 public:
    void        Read(const std::string& sequence);
    bool        HasParsedMessage() const;
    Error       GetErrorStatus() const;
    Request     GetParsedMessage();
    std::string FlushRemainder(); // TODO:(handrow)
};

}  // namespace Http

#endif  // HTTP_READER_H_
