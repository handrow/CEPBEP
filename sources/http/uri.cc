#include "common/fsm.h"
#include "http/uri.h"

namespace Http {

namespace {

typedef std::map<char, int> SymMap;

std::string  StrToLower(const std::string& str) {
    std::string low_str;
    for (usize i = 0; i < str.length(); ++i)
        low_str += tolower(str[i]);
    return low_str;
}

inline int  HexSymToNum(char c) {
    static SymMap* hex_symbols;
    if (hex_symbols == NULL) {
        hex_symbols = new SymMap;
        (*hex_symbols)['0'] = 0;
        (*hex_symbols)['1'] = 1;
        (*hex_symbols)['2'] = 2;
        (*hex_symbols)['3'] = 3;
        (*hex_symbols)['4'] = 4;
        (*hex_symbols)['5'] = 5;
        (*hex_symbols)['6'] = 6;
        (*hex_symbols)['7'] = 7;
        (*hex_symbols)['8'] = 8;
        (*hex_symbols)['9'] = 9;
        (*hex_symbols)['A'] = 10;
        (*hex_symbols)['B'] = 11;
        (*hex_symbols)['C'] = 12;
        (*hex_symbols)['D'] = 13;
        (*hex_symbols)['E'] = 14;
        (*hex_symbols)['F'] = 15;
    }
    SymMap::iterator it = hex_symbols->find(c);
    if (it == hex_symbols->end())
        return -1;
    return it->second;
}

}  // namespace

bool  IsUnrsvdSym(char sym) {
    return (isalnum(sym)
            || sym == '-'
            || sym == '.'
            || sym == '~'
            || sym == '_');
}

bool  IsPathSafeSym(char sym) {
    return (IsUnrsvdSym(sym)
            || sym == '@'
            || sym == ':'
            || sym == '/');
}

std::string         PercentDecode(const std::string& encoded_str) {
    std::string decoded_str;
    for (usize i = 0; i < encoded_str.length(); ++i) {
        if (encoded_str[i] == '%') {
            int high_part = HexSymToNum(encoded_str[i + 1]);
            int low_part = HexSymToNum(encoded_str[i + 2]);
            if (high_part < 0 || low_part < 0)
                return "";
            char decoded_sym = static_cast<char>(u8(high_part) * 16 + u8(low_part));
            i += 2;
            decoded_str += decoded_sym;
        } else {
            decoded_str += encoded_str[i];
        }
    }
    return decoded_str;
}

std::string         URI::ToString() const {
    std::string uri_str;

    if (!this->hostname.empty()) {
        uri_str += "http://"
                +  ((!this->userinfo.empty()) ? this->userinfo + "@" : "")
                +  StrToLower(this->hostname);
    }
    uri_str += PercentEncode(this->path, IsPathSafeSym)
            + (!this->query_str.empty() ? "?" + this->query_str : "")
            + (!this->fragment.empty() ? "#" + this->fragment : "");

    return uri_str;
}

std::string         Query::ToString() const {
    std::string query_str;
    for (ParamMap::const_iterator it = param_map.begin();;) {
        query_str += (PercentEncode(it->first, IsUnrsvdSym)
                  + "="
                  + PercentEncode(it->second, IsUnrsvdSym));
        if (++it != param_map.end())
            query_str += "&";
        else
            break;
    }
    return query_str;
}

namespace {

Query::ParamPair    ParseParamPamPam(const std::string& key_val_str) {
    usize        param_delim = key_val_str.find_first_of("=");

    return Query::ParamPair(key_val_str.substr(0, param_delim),
                            key_val_str.substr(param_delim + 1, -1));
}

}  // namespace

Query               Query::Parse(const std::string& query_str, Error*) {
    Query       query;
    usize       tok_begin = 0;
    usize       tok_end = 0;

    for (;;) {
        if (tok_end >= query_str.length())
            break;

        tok_begin = query_str.find_first_not_of("&", tok_end);
        if (tok_begin == std::string::npos)
            break;

        tok_end = query_str.find_first_of("&", tok_begin);

        const std::string key_val_str = query_str.substr(tok_begin, tok_end - tok_begin);       
        query.param_map.insert(ParseParamPamPam(key_val_str));
    }
    return query;
}


namespace {

enum UriFsmStates {
    STT_INIT,
    STT_AUTH,
    STT_PATH,
    STT_QUERY,
    STT_FRAG,
    STT_END,

    STATES_NUM
};

struct UriFsmStateData {
    const std::string&      input_str;
    usize                   tok_begin;
    usize                   tok_end;

    bool                    run;

    URI*                    uri;
    Error*                  err;

    inline bool Run() const {
        return run;
    }

    inline std::string GetToken() const {
        return input_str.substr(tok_begin, GetTokLen());
    }

    inline usize GetTokLen() const {
        return tok_end - tok_begin;
    }

};

typedef FSM<UriFsmStateData, STATES_NUM> UriFsmParser;
typedef UriFsmParser::StateIdx           StateIdx;

static const UriFsmParser::TransitionStateFunc NO_ACTION = NULL;

static const char HTTP_SCHEME_STR[] = "http://";
static const usize HTTP_SCHEME_STRLEN = sizeof(HTTP_SCHEME_STR) - 1;

//// INIT STATE

StateIdx URI_FSM_InitTrigger(UriFsmStateData* d) {
    const std::string scheme = d->input_str.substr(0, HTTP_SCHEME_STRLEN);

    if (StrToLower(scheme) == HTTP_SCHEME_STR)
        return STT_AUTH;
    return STT_PATH;
}

void     URI_FSM_InitToAuth(UriFsmStateData* d) {
    d->tok_begin = d->tok_end = HTTP_SCHEME_STRLEN;
}

//// AUTH STATE

StateIdx URI_FSM_AuthTrigger(UriFsmStateData* d) {

    usize spec_idx = d->input_str.find_first_of("@/?#", d->tok_begin);
    StateIdx next_state = STT_END;

    if (spec_idx == std::string::npos) {
        d->tok_end = d->input_str.length();
    } else {
        switch (d->input_str[spec_idx]) {
            case '@':   d->tok_end = spec_idx; next_state = STT_AUTH;   break;
            case '/':   d->tok_end = spec_idx; next_state = STT_PATH;   break;
            case '?':   d->tok_end = spec_idx; next_state = STT_QUERY;  break;
            case '#':   d->tok_end = spec_idx; next_state = STT_FRAG;   break;
        }
    }

    return next_state;
}

void     URI_FSM_AddUserInfo(UriFsmStateData* d) {
    d->uri->userinfo = d->GetToken();
    d->tok_begin = ++d->tok_end;
}

void    URI_FSM_AddHost(UriFsmStateData* d) {
    d->uri->hostname = d->GetToken();
    d->tok_begin = d->tok_end;
}

//// PATH STATE
StateIdx URI_FSM_PathTrigger(UriFsmStateData* d) {
    usize spec_idx = d->input_str.find_first_of("?#", d->tok_begin);
    StateIdx next_state = STT_END;

    if (spec_idx == std::string::npos) {
        d->tok_end = d->input_str.length();
    } else {
        switch (d->input_str[spec_idx]) {
            case '?':   d->tok_end = spec_idx; next_state = STT_QUERY;  break;
            case '#':   d->tok_end = spec_idx; next_state = STT_FRAG;   break;
        }
    }

    return next_state;
}

void    URI_FSM_AddPath(UriFsmStateData* d) {
    d->uri->path = PercentDecode(d->GetToken());
    d->tok_begin = d->tok_end;
}

//// QUERY STATE
StateIdx URI_FSM_QueryTrigger(UriFsmStateData* d) {

    d->tok_begin = ++d->tok_end;

    usize spec_idx = d->input_str.find_first_of("#", d->tok_begin);
    StateIdx next_state;

    if (spec_idx == std::string::npos) {
        d->tok_end = d->input_str.length();
        next_state = STT_END;
    } else {
        d->tok_end = spec_idx;
        next_state = STT_FRAG;
    }

    return next_state;
}


void    URI_FSM_AddQuery(UriFsmStateData* d) {
    d->uri->query_str = d->GetToken();
    d->tok_begin = d->tok_end;
}

//// FRAGMENT STATE
StateIdx URI_FSM_FragTrigger(UriFsmStateData* d) {
    d->tok_begin = ++d->tok_end;
    d->tok_end = d->input_str.length();
    return STT_END;
}

void    URI_FSM_AddFrag(UriFsmStateData* d) {
    d->uri->fragment = d->GetToken();
    d->tok_begin = d->tok_end;
}

StateIdx URI_FSM_End(UriFsmStateData* d) {
    d->run = false;
    return STT_END;
}

} // namespace


URI          URI::Parse(const std::string& uri_str, Error* err) {

    static UriFsmParser::Triggers fsm_triggers = {
        /*  INIT */ URI_FSM_InitTrigger,
        /*  AUTH */ URI_FSM_AuthTrigger,
        /*  PATH */ URI_FSM_PathTrigger,
        /* QUERY */ URI_FSM_QueryTrigger,
        /*  FRAG */ URI_FSM_FragTrigger,
        /*   END */ URI_FSM_End
    };

    static UriFsmParser::TransitionsMatrix fsm_matrix = {
                   /*         INIT                    AUTH             PATH              QUERY               FRAG              END */
        /*  INIT */ {          NULL,   URI_FSM_InitToAuth,         NO_ACTION,              NULL,              NULL,             NULL},
        /*  AUTH */ {          NULL,  URI_FSM_AddUserInfo,   URI_FSM_AddHost,   URI_FSM_AddHost,   URI_FSM_AddHost,  URI_FSM_AddHost},
        /*  PATH */ {          NULL,                 NULL,              NULL,   URI_FSM_AddPath,   URI_FSM_AddPath,  URI_FSM_AddPath},
        /* QUERY */ {          NULL,                 NULL,              NULL,              NULL,  URI_FSM_AddQuery, URI_FSM_AddQuery},
        /*  FRAG */ {          NULL,                 NULL,              NULL,              NULL,              NULL,  URI_FSM_AddFrag},
        /*   END */ {          NULL,                 NULL,              NULL,              NULL,              NULL,             NULL},
    };

    URI             uri;
    UriFsmStateData parser_state_data = {.input_str = uri_str, .tok_begin = 0, .tok_end = 0, .run = true, .uri = &uri, .err = err};
    UriFsmParser    parser(&fsm_triggers, &fsm_matrix);

    parser.Process(&parser_state_data, STT_INIT);
    return uri;
}

}  // namespace Http
