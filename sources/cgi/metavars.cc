#include "cgi.h"

#include <iostream>

namespace Cgi {
namespace {

std::string  TransformToMetavarStyle(const std::string& hkey) {
    std::string transformed;

    transformed += "HTTP_";
    for (usize i = 0; i < hkey.length(); ++i) {
        if (hkey[i] == '-') {
            transformed += '_';
            ++i;
        }
        transformed += toupper(hkey[i]);
    }
    return transformed;
}

}  // namespace

Envs CastToEnvs(const CStringVec& vec) {
    return reinterpret_cast<Envs>(&vec[0]);
}

void Metavars::AddHttpHeaders(const Http::Headers& hdrs) {
    std::string hkey;
    std::string hval;

    Http::Headers::HeaderMap map = hdrs.GetMap();
    for (std::map<std::string, std::string>::iterator it = map.begin();
                                                      it != map.end(); ++it) {
        hkey = TransformToMetavarStyle(it->first);
        hval = it->second;
        __map.insert(Http::Headers::HeaderPair(hkey, hval));
    }
}

void Metavars::AddEnvs(const Envs& envs) {
    char* const* env_cstr = envs;
    usize tok_begin;
    usize tok_end;

    while (*env_cstr != NULL) {
        std::string env_str = *env_cstr;
        std::string k, v;
        tok_begin = tok_end = 0;
        env_str = Trim(env_str, ' ');

        tok_end = env_str.find_first_of('=');

        if (tok_end == std::string::npos) {
            k = env_str;
            v = "";
        } else {
            k = env_str.substr(tok_begin, tok_end - tok_begin);
            tok_begin = ++tok_end;
            tok_end = env_str.length();
            v = env_str.substr(tok_begin, tok_end - tok_begin);
        }

        if (!k.empty())
            __map[k] = v;

        ++env_cstr;
    }
}

void Metavars::AddMetavars(const Metavars& mtvrs) {
    for (std::map<std::string, std::string>::const_iterator it =  mtvrs.__map.begin();
                                                            it != mtvrs.__map.end(); ++it) {
        __map[it->first] = __map[it->second];
    }
}

CStringVec
Metavars::ToEnvVector() const {
    std::map<std::string, std::string> metamap(__map);
    CStringVec vec;

    /// Patch Content length
    std::map<std::string, std::string>::const_iterator it = metamap.find("HTTP_CONTENT_LENGTH");
    if (it != metamap.end()) {
        metamap["CONTENT_LENGTH"] = it->second;
    }

    for (std::map<std::string, std::string>::const_iterator it = metamap.begin();
                                                            it != metamap.end(); ++it) {
        C::string str(it->first + "=" + it->second);
        vec.push_back(str);
    }

    vec.push_back(NULL);
    return vec;
}

std::map<std::string, std::string>&
Metavars::GetMapRef() {
    return __map;
}

const std::map<std::string, std::string>&
Metavars::GetMapRef() const {
    return __map;
}

}  // namespace Cgi
