#include "cgi.h"

#include <iostream>

namespace Cgi {
namespace {

std::string  TransformToMetavarStyle(const std::string& headerKey) {
    std::string transformed;

    transformed += "HTTP_";
    for (USize i = 0; i < headerKey.length(); ++i) {
        if (headerKey[i] == '-') {
            transformed += '_';
            ++i;
        }
        transformed += toupper(headerKey[i]);
    }
    return transformed;
}

}  // namespace

Envs CastToEnvs(const CStringVec& vec) {
    return reinterpret_cast<Envs>(&vec[0]);
}

void Metavars::AddHttpHeaders(const Http::Headers& hdrs) {
    std::string headerKey;
    std::string headerValue;

    Http::Headers::HeaderMap map = hdrs.GetMap();
    for (std::map<std::string, std::string>::iterator it = map.begin();
                                                      it != map.end(); ++it) {
        headerKey = TransformToMetavarStyle(it->first);
        headerValue = it->second;
        Map_.insert(Http::Headers::HeaderPair(headerKey, headerValue));
    }
}

void Metavars::AddEnvs(const Envs& envs) {
    char* const* envCStr = envs;
    USize tokBegin;
    USize tokEnd;

    while (*envCStr != NULL) {
        std::string envStr = *envCStr;
        std::string k, v;
        tokBegin = tokEnd = 0;
        envStr = Trim(envStr, ' ');

        tokEnd = envStr.find_first_of('=');

        if (tokEnd == std::string::npos) {
            k = envStr;
            v = "";
        } else {
            k = envStr.substr(tokBegin, tokEnd - tokBegin);
            tokBegin = ++tokEnd;
            tokEnd = envStr.length();
            v = envStr.substr(tokBegin, tokEnd - tokBegin);
        }

        if (!k.empty())
            Map_[k] = v;

        ++envCStr;
    }
}

void Metavars::AddMetavars(const Metavars& mtvrs) {
    for (std::map<std::string, std::string>::const_iterator it =  mtvrs.Map_.begin();
                                                            it != mtvrs.Map_.end(); ++it) {
        Map_[it->first] = Map_[it->second];
    }
}

CStringVec
Metavars::ToEnvVector() const {
    std::map<std::string, std::string> metamap(Map_);
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
    return Map_;
}

const std::map<std::string, std::string>&
Metavars::GetMapRef() const {
    return Map_;
}

}  // namespace Cgi
