#include "common/platform.h"
#include "common/fsm.h"
#include "http/uri.h"

namespace Http {

namespace {

typedef std::map<char, int> SymMap;

std::string  StrToLower(const std::string& str) {
    std::string lowStr;
    for (USize i = 0; i < str.length(); ++i)
        lowStr += tolower(str[i]);
    return lowStr;
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
    return  IsUnrsvdSym(sym)
            || sym == '@'
            || sym == ':'
            || sym == '/';
}

bool  IsSubDelim(char sym) {
    return  sym == '!' || sym == '$' ||
            sym == '&' || sym == '\''||
            sym == '(' || sym == ')' ||
            sym == '*' || sym == '+' ||
            sym == ',' || sym == ';' ||
            sym == '=';
}

bool  IsHostSafeSym(char sym) {
    return IsUnrsvdSym(sym) || IsSubDelim(sym) || sym == ':';
}

bool  IsUserSafeSym(char sym) {
    return IsUnrsvdSym(sym) || IsSubDelim(sym) || sym == ':';
}

std::string         PercentDecode(const std::string& encoded_str) {
    std::string decoded;
    for (USize i = 0; i < encoded_str.length(); ++i) {
        if (encoded_str[i] == '%') {
            int high_part = HexSymToNum(encoded_str[i + 1]);
            int low_part = HexSymToNum(encoded_str[i + 2]);
            if (high_part < 0 || low_part < 0)
                return "";
            char decodedSym = static_cast<char>(UInt8(high_part) * 16 + UInt8(low_part));
            i += 2;
            decoded += decodedSym;
        } else {
            decoded += encoded_str[i];
        }
    }
    return decoded;
}

std::string         URI::ToString() const {
    std::string uriString;

    if (!Hostname.empty()) {
        uriString += "http://"
                  +  ((!UserInfo.empty()) ? PercentEncode(UserInfo, IsUserSafeSym) + "@" : "")
                  +  PercentEncode(StrToLower(Hostname), IsHostSafeSym);
    }
    uriString += PercentEncode(Path, IsPathSafeSym)
              + (!QueryStr.empty() ? "?" + QueryStr : "")
              + (!Fragment.empty() ? "#" + Fragment : "");

    return uriString;
}

std::string         Query::ToString() const {
    std::string queryStr;
    for (ParamMap::const_iterator it = Params.begin();;) {
        queryStr += (PercentEncode(it->first, IsUnrsvdSym)
                  + "="
                  + PercentEncode(it->second, IsUnrsvdSym));
        if (++it != Params.end())
            queryStr += "&";
        else
            break;
    }
    return queryStr;
}

namespace {

Query::ParamPair    ParseParamPamPam(const std::string& keyValStr) {
    USize        paramDelim = keyValStr.find_first_of("=");

    return Query::ParamPair(PercentDecode(keyValStr.substr(0, paramDelim)),
                            PercentDecode(keyValStr.substr(paramDelim + 1, -1)));
}

}  // namespace

Query               Query::Parse(const std::string& queryStr, Error*) {
    Query       query;
    USize       tok_begin = 0;
    USize       tok_end = 0;

    for (;;) {
        if (tok_end >= queryStr.length())
            break;

        tok_begin = queryStr.find_first_not_of("&", tok_end);
        if (tok_begin == std::string::npos)
            break;

        tok_end = queryStr.find_first_of("&", tok_begin);

        const std::string keyValStr = queryStr.substr(tok_begin, tok_end - tok_begin);
        query.Params.insert(ParseParamPamPam(keyValStr));
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
    const std::string&      InputStr;
    USize                   TokBegin;
    USize                   TokEnd;

    bool                    Run;

    URI*                    Uri;
    Error*                  Err;

    inline bool GetRun() const {
        return Run;
    }

    inline std::string GetToken() const {
        return InputStr.substr(TokBegin, GetTokLen());
    }

    inline USize GetTokLen() const {
        return TokEnd - TokBegin;
    }
};

typedef FSM<UriFsmStateData, STATES_NUM> UriFsmParser;
typedef UriFsmParser::StateIdx           StateIdx;

static const UriFsmParser::TransitionStateFunc NO_ACTION = NULL;

static const char HTTP_SCHEME_STR[] = "http://";
static const USize HTTP_SCHEME_STRLEN = sizeof(HTTP_SCHEME_STR) - 1;

//// INIT STATE

StateIdx URI_FSM_InitTrigger(UriFsmStateData* d) {
    const std::string scheme = d->InputStr.substr(0, HTTP_SCHEME_STRLEN);

    if (StrToLower(scheme) == HTTP_SCHEME_STR)
        return STT_AUTH;
    return STT_PATH;
}

void     URI_FSM_InitToAuth(UriFsmStateData* d) {
    d->TokBegin = d->TokEnd = HTTP_SCHEME_STRLEN;
}

//// AUTH STATE

StateIdx URI_FSM_AuthTrigger(UriFsmStateData* d) {

    USize specIdx = d->InputStr.find_first_of("@/?#", d->TokBegin);
    StateIdx nextState = STT_END;

    if (specIdx == std::string::npos) {
        d->TokEnd = d->InputStr.length();
    } else {
        switch (d->InputStr[specIdx]) {
            case '@':   d->TokEnd = specIdx; nextState = STT_AUTH;   break;
            case '/':   d->TokEnd = specIdx; nextState = STT_PATH;   break;
            case '?':   d->TokEnd = specIdx; nextState = STT_QUERY;  break;
            case '#':   d->TokEnd = specIdx; nextState = STT_FRAG;   break;
        }
    }

    return nextState;
}

bool    IsValidPercentEncoding(const std::string& str, USize* i) {
    return (str.length() > *i && str.length() - *i > 3
            && str[*i] == '%'
            && ishexnumber(str[++(*i)])
            && ishexnumber(str[++(*i)]));
}

void    ValidateUserInfo(const std::string& userInfoStr, Error* err) {
    for (USize i = 0; i < userInfoStr.length(); ++i) {
        if (!IsUnrsvdSym(userInfoStr[i]) && !IsSubDelim(userInfoStr[i])
            && userInfoStr[i] != ':' && !IsValidPercentEncoding(userInfoStr, &i)
        ) {
            *err = Error(URI_BAD_USERINFO, "Bad user info syntax");
            break;
        }
    }
}

void     URI_FSM_AddUserInfo(UriFsmStateData* d) {
    d->Uri->UserInfo = d->GetToken();
    ValidateUserInfo(d->Uri->UserInfo, d->Err);
    if (d->Err->IsError())
        d->Run = false;
    d->Uri->UserInfo = PercentDecode(d->Uri->UserInfo);
    d->TokBegin = ++d->TokEnd;
}

void    ValidateHost(const std::string& hostStr, Error *err) {
    USize i = 0;
    for (; i < hostStr.length() && hostStr[i] != ':'; ++i) {
        if (!IsUnrsvdSym(hostStr[i]) && !IsSubDelim(hostStr[i]) && !IsValidPercentEncoding(hostStr, &i)) {
            *err = Error(URI_BAD_HOST, "Bad host syntax");
            return;
        }
    }

    if (i == 0) {  // if it's true, then it meanse that we have an empty hostname
        *err = Error(URI_BAD_HOST, "Empty host");
        return;
    }

    if (i < hostStr.length()) {  // if it's true, then it means that we stopped on the ':'
        USize j = i + 1;
        for (; j < hostStr.length(); ++j) {
            if (!isdigit(hostStr[j])) {
                *err = Error(URI_BAD_HOST, "Bad port syntax");
                return;
            }
        }
        if (j == i + 1) {  // if it's true, then it meanse that we have an empty hostname
            *err = Error(URI_BAD_HOST, "Bad port syntax");
            return;
        }
    }
}

void    URI_FSM_AddHost(UriFsmStateData* d) {
    d->Uri->Hostname = d->GetToken();
    ValidateHost(d->Uri->Hostname, d->Err);
    if (d->Err->IsError())
        d->Run = false;
    d->Uri->Hostname = StrToLower(PercentDecode(d->Uri->Hostname));
    d->TokBegin = d->TokEnd;
}

//// PATH STATE
StateIdx URI_FSM_PathTrigger(UriFsmStateData* d) {
    USize specIdx = d->InputStr.find_first_of("?#", d->TokBegin);
    StateIdx nextState = STT_END;

    if (specIdx == std::string::npos) {
        d->TokEnd = d->InputStr.length();
    } else {
        switch (d->InputStr[specIdx]) {
            case '?':   d->TokEnd = specIdx; nextState = STT_QUERY;  break;
            case '#':   d->TokEnd = specIdx; nextState = STT_FRAG;   break;
        }
    }

    return nextState;
}

void    ValidatePath(const std::string& pathStr, Error* err) {
    if (!pathStr.empty()) {
        USize segBegin = 0;
        USize segEnd = 0;
        while (segBegin < pathStr.length()) {
            if (pathStr[segBegin] == '/') {
                for (segEnd = segBegin + 1;
                    segEnd < pathStr.length() && pathStr[segEnd] != '/';
                    ++segEnd
                ) {
                    if (!IsUnrsvdSym(pathStr[segEnd]) && !IsSubDelim(pathStr[segEnd])
                        && pathStr[segEnd] != ':' && pathStr[segEnd] != '@'
                        && !IsValidPercentEncoding(pathStr, &segEnd)
                    ) {
                        *err = Error(URI_BAD_PATH_SYNTAX, "Bad path syntax");
                        return;
                    }
                }
                if (segEnd - segBegin <= 1 && segEnd < pathStr.length()) {
                    *err = Error(URI_BAD_PATH_SYNTAX, "Empty path segment");
                    return;
                }
                segBegin = segEnd;
            } else {
                *err = Error(URI_BAD_PATH_SYNTAX, "No '/' at path segment begining");
                return;
            }
        }
    }
}

void    URI_FSM_AddPath(UriFsmStateData* d) {
    d->Uri->Path = d->GetToken();
    ValidatePath(d->Uri->Path, d->Err);
    if (d->Err->IsError())
        d->Run = false;
    d->Uri->Path = PercentDecode(d->Uri->Path);
    d->TokBegin = d->TokEnd;
}

//// QUERY STATE
StateIdx URI_FSM_QueryTrigger(UriFsmStateData* d) {

    d->TokBegin = ++d->TokEnd;

    USize specIdx = d->InputStr.find_first_of("#", d->TokBegin);
    StateIdx nextState;

    if (specIdx == std::string::npos) {
        d->TokEnd = d->InputStr.length();
        nextState = STT_END;
    } else {
        d->TokEnd = specIdx;
        nextState = STT_FRAG;
    }

    return nextState;
}

void    URI_FSM_AddQuery(UriFsmStateData* d) {
    d->Uri->QueryStr = d->GetToken();
    d->TokBegin = d->TokEnd;
}

//// FRAGMENT STATE
StateIdx URI_FSM_FragTrigger(UriFsmStateData* d) {
    d->TokBegin = ++d->TokEnd;
    d->TokEnd = d->InputStr.length();
    return STT_END;
}

void    URI_FSM_AddFrag(UriFsmStateData* d) {
    d->Uri->Fragment = d->GetToken();
    d->TokBegin = d->TokEnd;
}

StateIdx URI_FSM_End(UriFsmStateData* d) {
    d->Run = false;
    if (d->Uri->Path.empty())
        d->Uri->Path = "/";
    return STT_END;
}

}  // namespace


URI          URI::Parse(const std::string& uriString, Error* err) {

    static UriFsmParser::Triggers FsmTriggers = {
        /*  INIT */ URI_FSM_InitTrigger,
        /*  AUTH */ URI_FSM_AuthTrigger,
        /*  PATH */ URI_FSM_PathTrigger,
        /* QUERY */ URI_FSM_QueryTrigger,
        /*  FRAG */ URI_FSM_FragTrigger,
        /*   END */ URI_FSM_End
    };

    static UriFsmParser::TransitionsMatrix FsmMatrix = {
                   /*         INIT                    AUTH             PATH              QUERY               FRAG              END */
        /*  INIT */ {          NULL,   URI_FSM_InitToAuth,         NO_ACTION,              NULL,              NULL,             NULL},
        /*  AUTH */ {          NULL,  URI_FSM_AddUserInfo,   URI_FSM_AddHost,   URI_FSM_AddHost,   URI_FSM_AddHost,  URI_FSM_AddHost},
        /*  PATH */ {          NULL,                 NULL,              NULL,   URI_FSM_AddPath,   URI_FSM_AddPath,  URI_FSM_AddPath},
        /* QUERY */ {          NULL,                 NULL,              NULL,              NULL,  URI_FSM_AddQuery, URI_FSM_AddQuery},
        /*  FRAG */ {          NULL,                 NULL,              NULL,              NULL,              NULL,  URI_FSM_AddFrag},
        /*   END */ {          NULL,                 NULL,              NULL,              NULL,              NULL,             NULL},
    };

    URI             Uri;
    UriFsmStateData ParserStateData = {.InputStr = uriString, .TokBegin = 0, .TokEnd = 0, .Run = true, .Uri = &Uri, .Err = err};
    UriFsmParser    Parser(&FsmTriggers, &FsmMatrix);

    Parser.Process(&ParserStateData, STT_INIT);
    return Uri;
}

}  // namespace Http
