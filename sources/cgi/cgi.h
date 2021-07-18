#ifndef CGI_CGI_H_
#define CGI_CGI_H_

#include <cstring>
#include <cstdlib>
#include <vector>

#include "http/http.h"

namespace C {

class string {
 private:
    char* __data;

 public:
    ~string() { delete[] __data; }

    string& operator=(const string& source) {
        if (&source != this) {
            if (source.__data == NULL)
                __data = NULL;
            else
                __data = ::strdup(source.__data);
        }
        return *this;
    }

    string(const string& source)
    : __data( NULL ) {
        if (source.__data == NULL)
            __data = NULL;
        else
            __data = ::strdup(source.__data);
    }

    string()
    : __data( NULL ) {
    }

    string(const std::string& str) // NOLINT(*)
    : __data( ::strdup(str.c_str()) ) {
    }

    string(const char* str) { // NOLINT(*)
        if (str == NULL) {
            __data = NULL;
        } else {
            __data = ::strdup(str);
        }
    }

    operator std::string() const {
        return std::string(__data == NULL ? "" : __data);
    }

    operator const char*() const {
        return __data;
    }
};

}  // namespace C

namespace Cgi {

typedef std::vector<C::string> CStringVec;
typedef char* const* Envs;

Envs CastToEnvs(const CStringVec& vec);

class Metavars {
 private:
    std::map<std::string, std::string>  __map;

 public:
    void AddHttpHeaders(const Http::Headers& hdrs);
    void AddEnvs(const Envs& envs);
    void AddMetavars(const Metavars& mtvrs);

    std::map<std::string, std::string>& GetMapRef();
    const std::map<std::string, std::string>& GetMapRef() const;

    CStringVec ToEnvVector() const;
};

}  // namespace Cgi

#endif  // CGI_CGI_H_
