#include "http/reader.h"

namespace Http {

namespace {

usize IsWhiteSpace(const std::string& s, usize i) {
    return (s[i] == ' ') ? 1
                         : 0;
}

usize IsCRLF(const std::string& s, usize i) {
    if (s[i] == '\r' && s[i + 1] == '\n')   return 2;
    if (s[i] == '\n')                       return 1;
    else                                    return 0;
}

usize IsHeaderKeySym(const std::string& s, usize i) {
    return !IsWhiteSpace(s, i); // TODO(handrow): Check for valid header key syms
}

usize IsHeaderValSym(const std::string&, usize) {
    return true; // TODO(handrow): Check for valid header value syms
}

}  // namespace

RequestReader::State
RequestReader::STT_INIT_Handler(bool*) {
    State next_state = STT_INIT;
    usize ws_skiper = IsWhiteSpace(__remainder, __i);
    usize crlf_skiper = IsCRLF(__remainder, __i);

    if (ws_skiper != 0) {
        __i += ws_skiper;
    } else if (crlf_skiper != 0) {
        __i += crlf_skiper;
    } else {
        next_state = STT_METHOD;
        __FlushParsedData();
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_METHOD_Handler(bool* run) {
    State next_state = STT_METHOD;

    if (IsWhiteSpace(__remainder, __i)) {
        next_state = STT_METHOD_END;
        __req.method = MethodFromString(__GetParsedData());
        __FlushParsedData();

        // Validate Method
        if (__req.method == METHOD_UNKNOWN) {
            __err = Error(HTTP_READER_BAD_METHOD, "Bad method syntax");
            next_state = STT_ERR;
            *run = false;
        }

    } else {
        __i += 1;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_METHOD_END_Handler(bool*) {
    State next_state = STT_METHOD_END;

    if (IsWhiteSpace(__remainder, __i)) {
        __i += 1;
    } else {
        next_state = STT_URI;
        __FlushParsedData();
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_URI_Handler(bool* run) {
    State next_state = STT_URI;

    if (IsWhiteSpace(__remainder, __i)) {
        next_state = STT_URI_END;

        Error err(0);
        __req.uri = URI::Parse(__GetParsedData(), &err);
        __FlushParsedData();

        // Check for URI parser errors
        if (err.IsError()) {
            next_state = STT_ERR;
            __err = err;
            *run = false;
        }
    } else {
        __i += 1;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_URI_END_Handler(bool*) {
    State next_state = STT_URI_END;

    if (IsWhiteSpace(__remainder, __i)) {
        __i += 1;
    } else {
        next_state = STT_VER;
        __FlushParsedData();
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_VER_Handler(bool* run) {
    State next_state = STT_VER;
    usize ws_skip = IsWhiteSpace(__remainder, __i);
    usize crlf_skip = IsCRLF(__remainder, __i);

    if (ws_skip > 0 || crlf_skip > 0) {
        next_state = (ws_skip > 0) ? STT_VER_END
                                   : STT_HDR_BEGIN;

        __req.version = ProtocolVersionFromString(__GetParsedData());

        __i += ws_skip + crlf_skip;  // only one of them isn't zero
        __FlushParsedData();


        if (__req.version == HTTP_NO_VERSION) {
            next_state = STT_ERR;
            __err = Error(HTTP_READER_BAD_PROTOCOL_VERSION, "Bad protocol version");
            *run = false;
        }
    } else {
        __i += 1;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_VER_END_Handler(bool* run) {
    State next_state = STT_VER_END;
    usize ws_skip = IsWhiteSpace(__remainder, __i);
    usize crlf_skip = IsCRLF(__remainder, __i);

    if (ws_skip != 0) {
        __i += ws_skip;
    } else if (crlf_skip != 0) {
        __i += crlf_skip;
        __FlushParsedData();
        next_state = STT_HDR_BEGIN;
    } else {
        // If it is not whitespace or new line, then it other symbol that is bad
        next_state = STT_ERR;
        __err = Error(HTTP_READER_BAD_TOKEN_IN_REQLINE, "Unknown token at the Req-Line end");
        *run = false;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_HDR_BEGIN_Handler(bool*) {
    State next_state = STT_HDR_BEGIN;
    usize ws_skip = IsWhiteSpace(__remainder, __i);
    usize crlf_skip = IsCRLF(__remainder, __i);

    if (ws_skip != 0) {
        __i += ws_skip;
    } else if (crlf_skip != 0) {
        next_state = STT_META_END;
        __i += crlf_skip;
    } else {
        next_state = STT_HDR_K;
        __FlushParsedData();
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_HDR_K_Handler(bool* run) {
    State next_state = STT_HDR_K;

    if (__remainder[__i] == ':') {
        next_state = STT_HDR_K_END;
        __hdr_key_tmp = __GetParsedData();
        __i += 1;
        __FlushParsedData();
    } else if (!IsHeaderKeySym(__remainder, __i)) {
        next_state = STT_ERR;
        __err = Error(HTTP_READER_BAD_HEADER_KEY);
        *run = false;
    } else {
        __i += 1;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_HDR_K_END_Handler(bool*) {
    State next_state = STT_HDR_K_END;

    if (IsWhiteSpace(__remainder, __i)) {
        __i += 1;
    } else {
        next_state = STT_HDR_V;
        __FlushParsedData();
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_HDR_V_Handler(bool* run) {
    State next_state = STT_HDR_V;
    usize crlf_skip = IsCRLF(__remainder, __i);

    if (crlf_skip != 0) {
        next_state = STT_HDR_V_END;
        __hdr_val_tmp = __GetParsedData(); // TODO(handow): trim right whitespace;
        __FlushParsedData();
        __i += crlf_skip;
    } else if (!IsHeaderValSym(__remainder, __i)) {
        next_state = STT_ERR;
        __err = Error(HTTP_READER_BAD_HEADER_VALUE);
        *run = false;
    } else {
        __i += 1;
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_HDR_V_END_Handler(bool*) {
    State next_state = STT_HDR_BEGIN;
    __req.headers.__map[__hdr_key_tmp] = __hdr_val_tmp;
    return next_state;
}

RequestReader::State
RequestReader::STT_META_END_Handler(bool*) {
    State next_state = STT_END;

    if (__req.method == METHOD_POST)
        next_state = STT_BODY_CL;

    __FlushParsedData();
    return next_state;
}

RequestReader::State
RequestReader::STT_BODY_CL_Handler(bool* run) {
    State next_state = STT_BODY_CL;
    usize content_length = Headers::GetContentLength(__req.headers);

    if (__remainder.length() >= content_length) {
        *run = false;
        next_state = STT_END;
        __i = content_length;
        __req.body = __GetParsedData();
        __FlushParsedData();
    }

    return next_state;
}

RequestReader::State
RequestReader::STT_ERR_Handler(bool*) {
    __err = Error(0, "no error");
    return STT_INIT;
}

RequestReader::State
RequestReader::STT_END_Handler(bool* run) {
    State next_state = STT_END;

    if (!HasParsedMessage())
        next_state = STT_INIT;

    *run = false;
    return next_state;
}


void        RequestReader::__Parse() {
    bool run = true;
    while (run) {
        if (__remainder[__i] == '\0') // TODO(handrow): BE SMART!
            break;
        switch (__state) {
            case STT_INIT:          __state = STT_INIT_Handler(&run);       break;
            case STT_METHOD:        __state = STT_METHOD_Handler(&run);     break;
            case STT_METHOD_END:    __state = STT_METHOD_END_Handler(&run); break;
            case STT_URI:           __state = STT_URI_Handler(&run);        break;
            case STT_URI_END:       __state = STT_URI_END_Handler(&run);    break;
            case STT_VER:           __state = STT_VER_Handler(&run);        break;
            case STT_VER_END:       __state = STT_VER_END_Handler(&run);    break;
            case STT_HDR_BEGIN:     __state = STT_HDR_BEGIN_Handler(&run);  break;
            case STT_HDR_K:         __state = STT_HDR_K_Handler(&run);      break;
            case STT_HDR_K_END:     __state = STT_HDR_K_END_Handler(&run);  break;
            case STT_HDR_V:         __state = STT_HDR_V_Handler(&run);      break;
            case STT_HDR_V_END:     __state = STT_HDR_V_END_Handler(&run);  break;
            case STT_META_END:      __state = STT_META_END_Handler(&run);   break;
            case STT_BODY_CL:       __state = STT_BODY_CL_Handler(&run);    break;
            case STT_ERR:           __state = STT_ERR_Handler(&run);        break;
            case STT_END:           __state = STT_END_Handler(&run);        break;
        }
    }
}

std::string RequestReader::__GetParsedData() const {
    return __remainder.substr(0, __i);
}

void        RequestReader::__FlushParsedData() {
    __remainder = __remainder.substr(__i);
    __i = 0;
}

void        RequestReader::Read(const std::string& sequence) {
    __remainder += sequence;
    if (!HasParsedMessage())
        __Parse();
}

bool        RequestReader::HasParsedMessage() const {
    return __state == STT_END;
}

Error       RequestReader::GetErrorStatus() const {
    return __err;
}

Request     RequestReader::GetParsedMessage() {
    Request req;
    if (HasParsedMessage()) {
        __state = STT_INIT;
        req = __req;
    }
    return req;
}

}  // namespace Http