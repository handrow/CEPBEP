#include "http/uri.h"

namespace Http {

inline static
bool        IsUnrsvdSym(char sym) {
    return (isalnum(sym)
            || sym == '-'
            || sym == '.'
            || sym == '~'
            || sym == '_');
}

//TODO:(handrow) find out about encoding !#%^&()=+  

inline static
std::string StrToLower(const std::string& str) {
    std::string low_str;
    for (usize i = 0; i < str.length(); ++i)
        low_str += tolower(str[i]);
    return low_str;
}

inline static
std::string ByteToHexStr(u8 byte) {
    static const char* hex_alphabet = "0123456789ABCDEF";
    return std::string(1, hex_alphabet[byte / 16])
            + std::string(1, hex_alphabet[byte % 16]);
}

typedef std::map<char, int> SymMap;

inline static
int         HexSymToNum(char c) {
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

std::string         URI::PercentEncode(const std::string& decoded_str) {
    std::string encoded_str;
    for (usize sym_idx = 0; sym_idx < decoded_str.length(); ++sym_idx) {
        if (!IsUnrsvdSym(decoded_str[sym_idx]))
            encoded_str += ("%" + ByteToHexStr(decoded_str[sym_idx]));
        else
            encoded_str += decoded_str[sym_idx];
    }
    return encoded_str;
}

std::string         URI::PercentDecode(const std::string& encoded_str) {
    std::string decoded_str;
    for (usize i = 0; i < encoded_str.length(); ++i) {
        if (encoded_str[i] == '%') {
            int high_part = HexSymToNum(encoded_str[i + 1]);
            int low_part = HexSymToNum(encoded_str[i + 2]);
            if (high_part < 0 || low_part < 0)
                return "";
            char decoded_sym = char(u8(high_part) * 16 + u8(low_part));
            i += 2;
            decoded_str += decoded_sym;
        }
        else
            decoded_str += encoded_str[i];
    }
    return decoded_str;
}

std::string         URI::EncodeScheme(Scheme scheme) {
    return scheme == URI_SCHEME_HTTP ? "http" : "";
}

URI::Scheme         URI::DecodeScheme(const std::string& str, Error* err) {
    std::string input_str = str;
    if (StrToLower(str) == "http")
        return URI_SCHEME_HTTP;
    return *err = Error(30001, "Unknown URI Scheme"), URI_SCHEME_UNKNOWN;
}

std::string         URI::EncodeAuthority(const Authority& auth) {
    std::string auth_str;
    if (!(auth.__username.empty())) {
        auth_str += auth.__username;
        if (!(auth.__password.empty()))
            auth_str += ":" + auth.__password;
        auth_str += "@";
    }
    auth_str += auth.__hostname;
    if (!(auth.__port.empty()))
        auth_str += ":" + auth.__port;
    return auth_str;
}

// authority = [ userinfo "@" ] host [ ":" port ]
URI::Authority      URI::DecodeAuthority(const std::string& str, Error*) {
    Authority   auth;
    usize       tok_begin = 0;
    usize       tok_end = 0;

    // parse userinfo
    tok_end = str.find_first_of('@', tok_begin);
    if (tok_end == std::string::npos)
        tok_end = 0; // no userinfo
    else {
        usize tok_delim = str.find_first_of(':', tok_begin);
        if (tok_delim < tok_end)
            auth.__password = str.substr(tok_delim + 1, tok_end - (tok_delim + 1));
        else
            tok_delim = tok_end;
        auth.__username = str.substr(tok_begin, tok_delim - tok_begin);
        tok_begin = ++tok_end;
    }

    // parse host
    tok_end = str.find_first_of(':', tok_begin);
    // no ':' means default port
    if (tok_end == std::string::npos) {
        tok_end = str.length();
        auth.__hostname = StrToLower(str.substr(tok_begin, tok_end - tok_begin));
    }
    else {
        auth.__hostname = StrToLower(str.substr(tok_begin, tok_end - tok_begin));
        tok_begin = ++tok_end;

        tok_end = str.length();
        auth.__port = str.substr(tok_begin, tok_end - tok_begin);
    }
    return auth;
}

std::string         URI::EncodeQuery(const QueryMap& params) {
    std::string query_str;
    for (QueryMap::const_iterator it = params.begin();;) {
        query_str += ( PercentEncode(it->first)
                     + "="
                     + PercentEncode(it->second) );
        if (++it != params.end())
            query_str += "&";
        else
            break;
    }
    return query_str;
}

// query = *( pchar / "/" / "?" )
URI::QueryMap       URI::DecodeQuery(const std::string& query, Error *) {
    QueryMap    map;
    usize       key_begin = 0;
    usize       key_end = 0;
    usize       val_begin = 0;
    usize       val_end = 0;

    while ((key_end = query.find("=", key_begin)) != std::string::npos) {
        if ((val_begin = query.find_first_not_of('=', key_end)) == std::string::npos)
            break;
        val_end = query.find("&", val_begin);

        const std::string key_substr = PercentDecode(query.substr(key_begin, key_end - key_begin));
        const std::string val_substr = PercentDecode(query.substr(val_begin, val_end - val_begin));
        map[key_substr] = val_substr;

        key_begin = val_end;
        if (key_begin != std::string::npos)
            ++key_begin;
    }
    return map;
}

std::string         URI::EncodeUri(const URI& uri) {
    std::string encoded_str;

    if (uri.__scheme != URI_SCHEME_URL)
        encoded_str += EncodeScheme(uri.__scheme) + "://"
                    +  EncodeAuthority(uri.__auth);

    encoded_str += uri.__path;

    if (!uri.__query_params.empty())
        encoded_str += "?" + EncodeQuery(uri.__query_params);

    if (!uri.__fragment.empty())
        encoded_str += "#" + uri.__fragment;

    return encoded_str;
}

URI                 URI::DecodeUri(const std::string& uri_str, Error* err) {
    URI     uri;
    usize   tok_begin = 0;
    usize   tok_end = 0;

    tok_end = uri_str.find("://");
    // If we have no :// in our URI, we consider that it is just URL (no authority and no scheme)
    if (tok_end != std::string::npos) {
        uri.__scheme = DecodeScheme(uri_str.substr(tok_begin, tok_end - tok_begin), err);
        if (err->IsError())
            return uri;
        tok_begin = tok_end + 3; // init new begin

        // init authority end
        tok_end = uri_str.find_first_of("/?#", tok_begin);
        uri.__auth = DecodeAuthority(uri_str.substr(tok_begin, tok_end - tok_begin), err);
        tok_begin = tok_end;
    }
    else
        uri.__scheme = URI_SCHEME_URL;

    // init path end
    tok_end = uri_str.find_first_of("?#", tok_begin);
    uri.__path = uri_str.substr(tok_begin, tok_end - tok_begin);

    if (uri_str[tok_end] == '?') {
        tok_begin = ++tok_end;
        tok_end = uri_str.find_first_of("#", tok_begin);
        uri.__query_params = DecodeQuery(uri_str.substr(tok_begin, tok_end - tok_begin), err); // check percent encode
    }
    
    if (uri_str[tok_end] == '#') {
        tok_begin = ++tok_end;
        tok_end = uri_str.length();
        uri.__fragment = uri_str.substr(tok_begin, tok_end - tok_begin);
    }
    return uri;
}

}  // namespace Http
